package Drow::Deploy::Host;
use strict;
use File::Rsync;
use Exporter;
use Expect::Simple;
use Data::Dumper;

our @ISA = qw(Exporter);
our @EXPORT = qw();

sub new {
  (my $class, my $product_name, my $app_name, my $world_name, my $host_info, my $repo_user) = @_;

  (my $world_base_name = $world_name) =~ s/^world_//d;

  my $self = { repo => {}
               , repo_user => ( $repo_user || ("$world_base_name" . "_version" ) )
               , product_name => $product_name
               , world_name => $world_name
               , app_name => $app_name
               , ip => $host_info->{ip}
               , ssh_port => ( $host_info->{'ssh-port'} or 22 )
               , timeout => 1000
               , errors => []
               , versions => {}
               , using_users => {}
               , users => {}
               , groups => {}
               , Verbose => 0
             };

  foreach my $user_info ( @{ $host_info->{users} } ) {
    $self->{using_users}->{ $user_info->{user_name} } = { name => $user_info->{user_name} }
      if $user_info->{world_name} eq $world_name;
  }

  #print Dumper($self->{using_users});

  bless $self, ref $class || $class;
  return $self;
}

sub sync_env {
  (my $self) = @_;

  my $repo_user_name = $self->{repo_user};
  my $repo_location = $repo_user_name . "@" . $self->{ip};
  my $data;

  my $exp = $self->start_expect($repo_location, $self->{ssh_port});
  return if not defined $exp;

  #读取用户组信息
  $exp->send( 'cat /etc/group' );
  my $groups = $self->{groups};
  ($data = $exp->before) =~ tr/\r//d;
  foreach my $line ( split( "\n", $data ) ) {
    if ($line =~ qr/^([\w\d]*)\:[\w\*\d]*\:([\w\d]*)\:([\w\d,]*)$/ ) {
      my $group = { id => $2, name => $1, users => [] };

      if ($3) {
        foreach my $group_user ( split( ',', $3) ) {
          push @{$group->{users}}, $group_user;
        }
      }

      $groups->{$2} = $group;
    }
  }

  #读取所有用户信息
  $exp->send( 'cat /etc/passwd' );
  ($data = $exp->before) =~ tr/\r//d;
  my $users = $self->{users};
  foreach my $line ( split( "\n", $data ) ) {
    if ($line =~ qr/^([\w\d\-]*)\:[\w]*\:([\d]*)\:([\d]*)\:[\w\d\-\s\(\)\,]*\:([\w\d\/\-]*)\:[\w\d\/]*$/ ) {
      $users->{$1} = { name => $1, user_id => $2, group_id => $3, home => $4 };
    }
  }

  #准备好repo_user相关信息
  $self->append_error("repo user $repo_user_name not exist in users!") and return if not exists $users->{$repo_user_name};
  my $repo_user_info = $users->{$repo_user_name};
  $self->append_error("repo user $repo_user_name group $repo_user_info->{group_id} exist in groups!") and return
    if not exists $groups->{$repo_user_info->{group_id}};
  my $repo_user_group = $groups->{$repo_user_info->{group_id}};

  #读取所有版本信息
  $exp->send( '/bin/ls -l' );
  ($data = $exp->before) =~ tr/\r//d;
  foreach my $line ( split( "\n", $data ) ) {
    if ($line =~ qr/([\w\d_-]+\-(\d+\.\d+\.\d+\.\d+))$/) {
      my $version = $2;
      my $version_path = $1;

      $self->append_error("version $version duplicate") and next if exists $self->{versions}->{$version};
      $self->{versions}->{$version} = { path => $repo_user_info->{home} . "/" . $version_path };
    }
  }

  #检测用户状态
  my $using_users = $self->{using_users};
  foreach my $using_user_name ( keys $using_users ) {
    my $using_user = $using_users->{$using_user_name};

    $self->append_error("user $using_user_name not exist in host $self->{ip}") and next
      if not exists $users->{$using_user_name};
    my $host_user = $users->{$using_user_name};

    if ( not grep { $_ eq $using_user_name } @{ $repo_user_group->{users} } ) {
      $self->append_error("user $using_user_name not exist in user group $repo_user_group->{name}, please use cmd 'usermod -a -G $repo_user_group->{name} $using_user_name'") and next
    }

    $using_user->{product_path} = $host_user->{home} . "/" . $self->{product_name};
    $using_user->{product_state} = "not-install";

    $exp->send( "/bin/ls -l $using_user->{product_path}" );
    ($data = $exp->before) =~ tr/\r//d;
    foreach my $line ( split( "\n", $data ) ) {
      if ($line =~ qr/(?: -> )([\w\s\d\-_\/\.]+)$/ ) {
        my $target_path = $1;

        $using_user->{installed_path} = $target_path;

        my $match = $repo_user_info->{home} . "/" . $self->{product_name} . "-";
        if ($target_path =~ qr/^(?:$match)(.*)$/) {
          $using_user->{product_state} = "installed";
          $using_user->{installed_version} = $1;
        }
        else {
          $using_user->{product_state} = "unknown-version";
        }
      }
    }
  }

  #读取运行状态
  # $exp->send( 'LC_ALL=C /bin/ps -eo "gid uid pid ppid start command" --columns 256' );
  # ($data = $exp->before) =~ tr/\r//d;
  # foreach my $line ( split( "\n", $data ) ) {
  #   if ($line =~ qr/^\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+([\d\:]+)\s+(.*)$/ ) {
  #     print "xxxx gid=$1, uid=$2, pid=$3, ppid=$4, start=$5: $6\n";
  #   }
  #   else {
  #     print "xxx $line\n";
  #   }
  # }
}

sub sync_env_sync_user {
  (my $self, my $user_name, my $target_version_name, my $set_type) = @_;

  #print Dumper($self->{using_users});

  $self->append_error("target $target_version_name not exist") and return
    if not exists $self->{versions}->{$target_version_name};
  my $target_version = $self->{versions}->{$target_version_name};

  $self->append_error("user $user_name is not using user") and return
    if not exists $self->{using_users}->{$user_name};
  my $using_user = $self->{using_users}->{$user_name};

  my $user_location = $using_user->{name} . "@" . $self->{ip};

  my $do_unlink = 0;

  if ( $using_user->{product_state} eq "not-install" ) {
  }
  elsif ( $using_user->{product_state} eq "installed" ) {
    return if ($using_user->{installed_version} eq $target_version_name);
    $do_unlink = 1;
  }
  elsif ( $using_user->{product_state} eq "unknown-version" ) {
    $do_unlink = 1;
  }

  my $exp = $self->start_expect($user_location, $self->{ssh_port});
  return if not defined $exp;

  if ($do_unlink) {
    $exp->send( "rm -rf $using_user->{product_path}" );
    $self->append_error("user $user_name unlink version $using_user->{product_path} fail") and return
      if $exp->error;
  }

  $exp->send( "/bin/ln -s $target_version->{path} $using_user->{product_path}" );
  $self->append_error("user $user_name link version $target_version->{path} ==> $using_user->{product_path} fail") and return
    if $exp->error;
}

sub center_execute {
  (my $self, my $center_info, my $op) = @_;

  #print Dumper($center_info);
  my $user_name = $center_info->{user};

  $self->append_error("user $user_name is not using user") and return
    if not exists $self->{using_users}->{$user_name};
  my $using_user = $self->{using_users}->{$user_name};

  my $user_location = $user_name . "@" . $self->{ip};
  my $exp = $self->start_expect($user_location, $self->{ssh_port});
  return if not defined $exp;

  my $center_cmd = $using_user->{product_path} . "/" . $self->{app_name} . "Center/bin/" . $self->{app_name} . "_center";

  if ($op eq "start") {
    return "skip" if ($self->is_progress_runing($exp, $user_name, $center_cmd));
    return $self->center_do_start($exp, $center_cmd, $center_info);
  }
  elsif ($op eq "stop") {
    return "skip" if (not $self->is_progress_runing($exp, $user_name, $center_cmd));
    return $self->center_do_stop($exp, $center_cmd, $center_info);
  }
  elsif ($op eq "restart") {
    if ($self->is_progress_runing($exp, $user_name, $center_cmd)) {
      my $stop_result = $self->center_do_stop($exp, $center_cmd, $center_info);
      return $stop_result if (not defined $stop_result or $stop_result ne "done");
      return "fail" if ( not $self->wait_progress_stop($exp, $user_name, $center_cmd) );
    }

    return $self->center_do_start($exp, $center_cmd, $center_info);
  }
  elsif ($op eq "status") {
    return $self->is_progress_runing($exp, $user_name, $center_cmd) ? "runing" : "stoped";
  }
}

sub center_do_start {
  (my $self, my $exp, my $center_cmd, my $center_info) = @_;

  my $center_cmd_start = "$center_cmd start  --shm-size=$center_info->{shm_size} --listen=$center_info->{port}";
  $exp->send($center_cmd_start);
  $self->append_error("start center fail, cmd: $center_cmd_start") and return
    if $exp->error;

  return "fail" if ( not $self->is_progress_runing($exp, $center_info->{user}, $center_cmd) );
  return "done";
}

sub center_do_stop {
  (my $self, my $exp, my $center_cmd, my $center_info) = @_;

  my $center_cmd_stop = "$center_cmd stop";
  $exp->send($center_cmd_stop);
  $self->append_error("stop center fail, cmd: $center_cmd_stop") and return
    if $exp->error;

  return "fail" if ( not $self->wait_progress_stop($exp, $center_info->{user}, $center_cmd) );
  return "done";
}

sub set_execute {
  (my $self, my $set_info, my $op) = @_;

  #print Dumper($center_info);
  my $user_name = $set_info->{location}->{user};

  $self->append_error("user $user_name is not using user") and return
    if not exists $self->{using_users}->{$user_name};
  my $using_user = $self->{using_users}->{$user_name};

  my $user_location = $user_name . "@" . $self->{ip};
  my $exp = $self->start_expect($user_location, $self->{ssh_port});
  return if not defined $exp;

  my $set_cmd = $using_user->{product_path} . "/" . $self->{app_name} . "Set/bin/" . $self->{app_name} . "_set";

  if ($op eq "start") {
    return "skip" if ($self->is_progress_runing($exp, $user_name, $set_cmd));
    return $self->set_do_start($exp, $set_cmd, $set_info);
  }
  elsif ($op eq "stop") {
    return "skip" if (not $self->is_progress_runing($exp, $user_name, $set_cmd));
    return $self->set_do_stop($exp, $set_cmd, $set_info);
  }
  elsif ($op eq "restart") {
    if ($self->is_progress_runing($exp, $user_name, $set_cmd)) {
      my $stop_result = $self->set_do_stop($exp, $set_cmd, $set_info);
      return $stop_result if (not defined $stop_result or $stop_result ne "done");
      return "fail" if ( not $self->wait_progress_stop($exp, $user_name, $set_cmd) );
    }

    return $self->set_do_start($exp, $set_cmd, $set_info);
  }
  elsif ($op eq "status") {
    return $self->is_progress_runing($exp, $user_name, $set_cmd) ? "runing" : "stoped";
  }
}

sub set_do_start {
  (my $self, my $exp, my $set_cmd, my $set_info) = @_;

  my $set_name = $set_info->{type} . "_" . $set_info->{id};
  my $set_cmd_start = "$set_cmd start --set-name=$set_name";

  $exp->send($set_cmd_start);
  $self->append_error("start set fail, cmd: $set_cmd_start") and return
    if $exp->error;
  #print $exp->before . "\n";

  return "fail" if ( not $self->is_progress_runing($exp, $set_info->{location}->{user}, $set_cmd) );
  return "done";
}

sub set_do_stop {
  (my $self, my $exp, my $set_cmd, my $set_info) = @_;

  my $set_name = $set_info->{type} . "_" . $set_info->{id};
  my $set_cmd_stop = "$set_cmd stop --set-name=$set_name";

  $exp->send($set_cmd_stop);
  $self->append_error("stop set fail, cmd: $set_cmd_stop") and return
    if $exp->error;
  #print $exp->before . "\n";

  return "fail" if ( not $self->wait_progress_stop($exp, $set_info->{location}->{user}, $set_cmd) );
  return "done";
}

sub export_version {
  (my $self, my $version, my $version_dir, my $version_user) = @_;

  #print "xxxxx ssh -p $self->{ssh_port}\n";

  my $rsync = File::Rsync->new(archive => 1
                               , compress => 1
                               , del=>1
                               , rsh => "ssh -p $self->{ssh_port}"
                               , exclude=> [ "log" ]
                              );

  my $repo_user = $self->{repo_user};
  my $version_path = $self->{product_name} . "-" . $version;
  my $repo_target = $repo_user . "@" . $self->{ip} . ":" . $version_path . "/";

  print "begin export $version_dir to $repo_target....";
  $rsync->exec( src => "$version_dir/", dest => $repo_target ) or die "rsync failed\n";
  print "done\n";

  my $repo_user_info = $self->{users}->{$repo_user};

  $self->{versions}->{$version} = { path => $repo_user_info->{home} . "/" . $version_path };
}

sub append_error {
  (my $self, my $msg) = @_;

  my $error = { msg => $msg };

  push @{ $self->{errors} }, $error;
}

sub have_error {
  my $self = shift;

  return @{ $self->{errors} } > 0;
}

sub print_error {
  my $self = shift;

  foreach my $err (@{ $self->{errors} }) {
    print $err->{msg} . "\n";
  }

  return 1;
}

sub start_expect {
  (my $self, my $location, my $port) = @_;

  #print "start expect $location $port\n";

  my $exp = new Expect::Simple
    { Cmd => [ "ssh", "-p", $port, $location ],
      Prompt => [ -re => '.*\@.*~.*\$\s+' ],
      DisconnectCmd => 'exit',
      Verbose => $self->{Verbose},
      Debug => 0,
      Timeout => $self->{timeout},
    };

  $self->append_error("ssh to $location fail") and return if not defined $exp;

  return $exp;
}

sub is_progress_runing {
  (my $self, my $exp, my $user_name, my $cmd) = @_;

  $exp->send("ps -fU $user_name -o command --columns=256");
  (my $data = $exp->before) =~ tr/\r//d;
  foreach my $line ( split( "\n", $data ) ) {
    if ($line =~ qr/^($cmd)/ ) {
      return 1;
    }
  }

  return 0;
}

sub wait_progress_stop {
  (my $self, my $exp, my $user_name, my $cmd) = @_;

  for (my $retry_count = 0; $retry_count < 3; ++$retry_count) {
    return 1 if (not $self->is_progress_runing($exp, $user_name, $cmd));
    sleep 1;
  }

  return 0;
}

1;
