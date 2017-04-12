package Drow::Deploy::DB::Parse;
use strict;
use Exporter;
use Data::Dumper;

our @ISA = qw(Exporter);
our @EXPORT = qw(parse_args parse_chanels parse_versions parse_hosts parse_dbs parse_worlds parse_regions parse_sets);

sub parse_args {
  (my $self, my $table) = @_;

  my $args = $self->{args};

  foreach my $record ( @{$table} ) {
    #name
    $self->append_error("arg name not configured", $record->{_location}) and next if (! exists $record->{name} );
    my $arg_name = $record->{name};
    $self->append_error("arg $arg_name already configured", $record->{_location}) and next if (exists $args->{$arg_name});

    #value
    $self->append_error("arg value not configured", $record->{_location}) and next if (! exists $record->{value} );
    my $arg_value = $record->{value};

    $args->{$arg_name} = $arg_value;
  }

  return 1;
}

sub parse_chanels {
  (my $self, my $table) = @_;

  my $chanels = $self->{chanels};

  foreach my $record ( @{$table} ) {
    #name
    $self->append_error("chanel not configured", $record->{_location}) and next if (! exists $record->{chanel} );
    my $chanel = $record->{chanel};
    $self->append_error("chanel $chanel already configured", $record->{_location}) and next if (grep { $_ eq $chanel } @{ $chanels });

    push @{ $chanels }, $chanel;
  }

  return 1;
}

sub parse_hosts {
  (my $self, my $table) = @_;

  my $hosts = $self->{hosts};

  foreach my $record ( @{$table} ) {
    #name
    $self->append_error("host name not configured", $record->{_location}) and next if (! exists $record->{name} );
    my $host_name = $record->{name};
    $self->append_error("host name $host_name not configured", $record->{_location}) and next if (exists $hosts->{$host_name});

    #ip
    $self->append_error("host $host_name ip not configured", $record->{_location}) and next if (not exists $record->{ip} );
    my $host_ip = $record->{ip};
    $self->append_error("host $host_name ip $host_ip format error", $record->{_location}) and next if ($host_ip !~ /^\d+\.\d+\.\d+\.\d+$/);
    $record->{internal_ip} = $record->{internal_ip} if exists $record->{internal_ip};

    $record->{ports} = {};
    $record->{worlds} = [];
    $record->{users} = [];

    $hosts->{$host_name} = $record;
  }

  return 1;
}

sub parse_versions {
  (my $self, my $table) = @_;

  my $versions = $self->{versions};

  my $cur_version;

  foreach my $record ( @{$table} ) {
    #version
    my $version_version = exists $record->{version} ? $record->{version} : "";

    my $version_desc = exists $record->{desc} ? $record->{desc} : "";
    my $version_strategy = exists $record->{strategy} ? $record->{strategy} : "";

    if ($version_version ne "") {
      $self->append_error("version version $version_version not configured", $record->{_location}) and next if (exists $versions->{$version_version});
      $self->append_error("version $version_version format error ", $record->{_location}) and next if $version_version !~ qr/^\d+\.\d+\.\d+\.\d+$/;
      $self->append_error("version $version_version desc not configured ", $record->{_location}) and next if $version_desc eq "";
      $self->append_error("version $version_version strategy not configured ", $record->{_location}) and next if $version_strategy eq "";

      $self->append_error("version stratery $version_strategy unknown, should be (force|advise|silence|hide)", $record->{_location}) and next
        if not grep { $_ eq $version_strategy } ('force', 'advise', 'silence', 'hide');


      $cur_version = {
                      version => $version_version,
                      desc => $version_desc,
                      strategy => $version_strategy,
                      packages => [],
                      '_location' => $record->{_location}
                     };

      $versions->{$version_version} = $cur_version;
    }
    else {
      $self->append_error("can`t configure desc in follow line", $record->{_location}) and next if $version_desc ne "";
      $self->append_error("can`t configure stratery in follow line", $record->{_location}) and next if $version_strategy ne "";
      $self->append_error("no curent version", $record->{_location}) and next if not defined $cur_version;
    }

    #chanel
    $self->append_error("version chanel not configured", $record->{_location}) and next if (! exists $record->{chanel} );
    my $version_chanel = $record->{chanel};

    #category
    $self->append_error("version category not configured", $record->{_location}) and next if (! exists $record->{category} );
    my $version_category = $record->{category};

    #size
    $self->append_error("version size not configured", $record->{_location}) and next if (! exists $record->{size} );
    my $version_size = $record->{size};

    #md5
    $self->append_error("version md5 not configured", $record->{_location}) and next if (! exists $record->{md5} );
    my $version_md5 = $record->{md5};

    #url
    $self->append_error("version url not configured", $record->{_location}) and next if (! exists $record->{url} );
    my $version_url = $record->{url};

    push @{ $cur_version->{packages} }, { chanel => $version_chanel,
                                          category => $version_category,
                                          size => $version_size,
                                          md5 => $version_md5,
                                          url => $version_url,
                                          };
  }

  return 1;
}

sub parse_dbs {
  (my $self, my $table) = @_;

  my $dbs = $self->{dbs};
  my $hosts = $self->{hosts};

  foreach my $record ( @{$table} ) {
    #name
    $self->append_error("db name not configured", $record->{_location}) and next if (! exists $record->{name} );
    my $db_name = $record->{name};
    $self->append_error("db name $db_name duplicate", $record->{_location}) and next if (exists $dbs->{$db_name});

    #location
    $self->append_error("db location not configured", $record->{_location}) and next if (! exists $record->{location} );
    my $db_port;
    my $db_ip;
    my $db_host_name;
    my $db_svr;

    my $db_location = $record->{location};
    if ($db_location =~ /([\d\w\._-]+):(\d+)$/m) {
      $db_port = $2;

      my $db_host = $1;
      if ($db_host =~ /^\d+\.\d+\.\d+\.\d+$/) {
        $db_ip = $db_host;
      }
      else {
        $self->append_error("db host $db_host not exist", $record->{_location}) and next if (! exists $hosts->{$db_host} );

        my $host = $hosts->{$db_host};

        host_add_port($self, $host, $db_port, "db", $record->{_location});

        $db_host_name = $db_host;
        $db_ip = exists $host->{internal_ip} ? $host->{internal_ip} : $host->{ip};
      }

      $db_svr = { ip => $db_ip, port => $db_port, worlds => [] };
    }
    elsif ($db_location =~ /^mongodb:\/\//m) {
      $db_svr = { uri => $db_location };
    }
    else {
      $self->append_error("db location $db_location format error", $record->{_location}) and next;
    }

    $record->{db_svr} = $db_svr;
    $dbs->{$db_name} = $record;
  }

  return 1;
}

sub parse_worlds {
  (my $self, my $table) = @_;

  my $worlds = $self->{worlds};
  my $hosts = $self->{hosts};

  foreach my $record ( @{$table} ) {
    #name
    $self->append_error("world name not configured", $record->{_location}) and next if (! exists $record->{name} );
    my $world_name = $record->{name};
    $self->append_error("world name $world_name duplicate", $record->{_location}) and next if (exists $worlds->{$world_name});

    #center-version
    $self->append_error("world center version not configured", $record->{_location}) and next if (! exists $record->{center_version} );
    my $center_version = $record->{center_version};

    #center-shm_size
    $self->append_error("world center shm_size not configured", $record->{_location}) and next if (! exists $record->{center_shm_size} );
    my $center_shm_size = $record->{center_shm_size};

    #center-port
    $self->append_error("world center port not configured", $record->{_location}) and next if (! exists $record->{center_port} );
    my $center_port = $record->{center_port};

    #center
    $self->append_error("world center not configured", $record->{_location}) and next if (! exists $record->{center} );
    my $center_host;
    my $center_user_name;
    my $world_center = $record->{center};
    if ($world_center =~ /^([\d\w_-]+)@([\d\w\._-]+)$/m) {
      my $center_host_name = $2;
      $center_user_name = $1;

      $self->append_error("world $world_name host $center_host_name not exist", $record->{_location}) and next if (! exists $hosts->{$center_host_name} );

      $center_host = $hosts->{$center_host_name};

      $record->{repo_user} = ( $record->{repo_user} || $world_name );

      $record->{center} = { user => $center_user_name
                            , host_name => $center_host_name
                            , host_ip => exists $center_host->{internal_ip} ? $center_host->{internal_ip} : $center_host->{ip}
                            , version => $center_version
                            , shm_size => $center_shm_size
                            , port => $center_port
                          };
    }
    else {
      $self->append_error("world center $world_center format error", $record->{_location}) and next;
    }

    host_add_port($self, $center_host, $center_port, "$world_name.center", $record->{_location});
    host_add_user($self, $center_host, $world_name, $center_user_name, $record->{_location});
    $record->{regions} = {};
    $record->{common_sets} = { sets => {} };
    $record->{sets} = {};

    $worlds->{$world_name} = $record;
  }

  return 1;
}

sub parse_regions {
  (my $self, my $table) = @_;

  my $regions = $self->{regions};
  my $worlds = $self->{worlds};
  my $chanels = $self->{chanels};

  foreach my $record ( @{$table} ) {
    #world
    $self->append_error("region world not configured", $record->{_location}) and next if (! exists $record->{world} );
    my $region_world = $record->{world};
    $self->append_error("region world $region_world not exist", $record->{_location}) and next if (! exists $worlds->{$region_world} );
    my $world = $worlds->{$region_world};
    my $world_regions = $world->{regions};

    #id
    $self->append_error("region id not configured", $record->{_location}) and next if (! exists $record->{id} );
    my $region_id = $record->{id};
    $self->append_error("region $region_world.$region_id already exist", $record->{_location}) and next if (exists $world_regions->{$region_id} );

    #state
    $self->append_error("region $region_world.$region_id state not configured", $record->{_location}) and next if (! exists $record->{state} );
    my $region_state = $record->{state};
    $self->append_error("region $region_world.$region_id state unknown, should be (public/testing/internal)", $record->{_location}) and next
      if (! grep { $_ eq $region_state } ( "public", "testing", "internal" ) );

    #desc
    $self->append_error("region desc not configured", $record->{_location}) and next if (! exists $record->{desc} || $record->{desc} =~ /^\s*$/ );
    my $region_desc = $record->{desc};

    #chanels
    $self->append_error("region chanels not configured", $record->{_location}) and next if (! exists $record->{chanels} || $record->{chanels} =~ /^\s*$/ );
    my @region_chanels = split /\s*,\s*/, $record->{chanels};
    foreach my $chanel ( @region_chanels ) {
      $self->append_error("region $region_world.$region_id chanel $chanel unknown", $record->{_location}) and next
        if (! grep { $_ eq $chanel } @{ $chanels } );
    }

    #categories
    $self->append_error("region categories not configured", $record->{_location}) and next if (! exists $record->{category} || $record->{category} =~ /^\s*$/ );
    my @region_categories = split /\s*,\s*/, $record->{category};
    foreach my $category ( @region_categories ) {
      $self->append_error("region $region_world.$region_id category $category unknown, should be (windows/ios/android)", $record->{_location}) and next
        if (! grep { $_ eq $category } ( "windows", "ios", "android" ) );
    }

    $record->{sets} = {};
    $record->{servers} = [];
    $record->{chanels} = \@region_chanels;
    $record->{'device-categories'} = \@region_categories;

    $world_regions->{$region_id} = $record;
    push @{ $regions }, $record;
  }

  return 1;
}

sub parse_sets {
  (my $self, my $table) = @_;

  my $args = $self->{args};
  my $region_set_type;
  my $region_port;

  $self->append_error("arg region-server not configured") if (not exists $args->{'region-server'});
  my $arg_region_server = $args->{'region-server'};
  if ($arg_region_server =~ /^(\w+)\.([\w-]+)/) {
    $region_set_type = $1;
    $region_port = $2;
  }
  else {
    $self->append_error("arg region-server " . $arg_region_server . " format error!");
  }

  my $sets = $self->{sets};
  my $hosts = $self->{hosts};
  my $dbs = $self->{dbs};
  my $regions = $self->{regions};
  my $worlds = $self->{worlds};

  foreach my $record ( @{$table} ) {
    #world
    $self->append_error("set world not configured", $record->{_location}) and next if (! exists $record->{world} );
    my $set_world = $record->{world};
    $self->append_error("set world $set_world not exist", $record->{_location}) and next if (! exists $worlds->{$set_world} );
    my $world = $worlds->{$set_world};
    my $world_regions = $world->{regions};
    my $world_sets = $world->{sets};

    #id
    $self->append_error("set id not configured", $record->{_location}) and next if (! exists $record->{id} );
    my $set_id = $record->{id};
    $self->append_error("set $set_world.$set_id already exist", $record->{_location}) and next if (exists $world_sets->{$set_id} );

    #type
    $self->append_error("set $set_world.$set_id type not configured", $record->{_location}) and next if (! exists $record->{type} || $record->{type} =~ /^\s*$/ );
    my $set_type = $record->{type};

    #version
    $self->append_error("set $set_world.$set_id version not configured", $record->{_location}) and next if (! exists $record->{version} || $record->{version} =~ /^\s*$/ );
    my $set_version = $record->{version};

    #region
    my $set_region_id;
    my $set_region;
    if ( exists $record->{region}  and $record->{region}) {
      $set_region_id = $record->{region};
      $self->append_error("set $set_world.$set_id region $set_region_id not exist", $record->{_location}) and next if (! exists $world_regions->{$set_region_id} );
      $set_region = $world_regions->{$set_region_id};
    }
    else {
      $set_region = $world->{common_sets};
      die "world $world->{name} no common_sets" if not defined $set_region;
    }

    my $region_sets = $set_region->{sets};

    #port
    $self->append_error("set $set_world.$set_id port not configured", $record->{_location}) and next if (! exists $record->{'set-port'} );
    my $set_port = $record->{'set-port'};

    #location
    $self->append_error("set $set_world.$set_id location not configured", $record->{_location}) and next if (! exists $record->{location} );
    my $set_location_name = $record->{location};
    my $set_host;
    my $set_user_name;
    my $set_location = { host_port => $set_port };

    if ($set_location_name =~ /^([\d\w_-]+)@([\d\w\._-]+)$/m) {
      $set_user_name = $1;
      $set_location->{user} = $set_user_name;

      my $set_host_name = $2;
      if ($set_host_name =~ /^\d+\.\d+\.\d+\.\d+$/) {
        $set_location->{host_ip} = $set_host_name;
        $set_location->{export_ip} = $set_host_name;
      }
      else {
        $self->append_error("set $set_world.$set_id host $set_host not exist", $record->{_location}) and next if (! exists $hosts->{$set_host_name} );

        $set_host = $hosts->{$set_host_name};

        $set_location->{host_name} = $set_host_name;
        $set_location->{host_ip} = exists $set_host->{internal_ip} ? $set_host->{internal_ip} : $set_host->{ip};
        $set_location->{export_ip} = $set_host->{ip};
      }
    } else {
      $self->append_error("set $set_world.$set_id location $set_location_name format error", $record->{_location}) and next;
    }

    my $addition_args = { 'region-id' => $set_region_id
                        };
    foreach my $arg_name ( keys $record ) {
      next if ( grep { $_ eq $arg_name } ('_location', 'world', 'id', 'name', 'location', 'set-port', 'type', 'region') );
      my $arg_value = $record->{$arg_name};
      next if (not $arg_value);

      if ($arg_name =~ /^([\w-]+)-db$/ or $arg_name eq "db") {
        my $db_arg_name = $1;
        my $db_ns;
        my $db_ins_name;

        if ($arg_value =~ /^([\w_]+)\|([\w\d_-]+)$/) {
          $db_ns = $1;
          $db_ins_name = $2;
        }
        else {
          $self->append_error("set $set_world.$set_id db $db_arg_name value $arg_value format error", $record->{_location});
          next;
        }

        $self->append_error("set $set_world.$set_id db $db_arg_name use ins $db_ins_name  not exist", $record->{_location}) and next
          if (! exists $dbs->{$db_ins_name} );
        my $db = $dbs->{$db_ins_name};

        $addition_args->{$arg_name . "-ns"} = $db_ns;
        if (defined $db->{db_svr}->{uri}) {
          $addition_args->{$arg_name . "-location"} = $db->{db_svr}->{uri};
        }
        else {
          $addition_args->{$arg_name . "-location"} = "mongodb://" . $db->{db_svr}->{ip} . ':' . $db->{db_svr}->{port};
        }
      }
      elsif ($arg_name =~ /^([\w-]+)-port$/) {
        host_add_port($self, $set_host, $arg_value, "$set_world.$set_type-$set_id.$1", $record->{_location});
        $addition_args->{$arg_name} = $arg_value;
      }
      elsif ($arg_name =~ /^([\w-]+)-location$/) {
        if ($arg_value =~ /^([\w\d_\-]+)\:(\d+)$/) {
          my $to_host_name = $1;
          my $to_host_port = $2;

          if ($to_host_name =~ /^\d+\.\d+\.\d+\.\d+$/) {
            $addition_args->{$arg_name} = $to_host_name . ":" . $to_host_port;
          } else {
            $self->append_error("$set_world.$set_id use host $to_host_name not exist", $record->{_location}) and next if (! exists $hosts->{$to_host_name} );

            my $to_host = $hosts->{$to_host_name};

            $addition_args->{$arg_name} = $to_host->{ip} . ":" . $to_host_port;
          }
        }
        else {
          $self->append_error("set $set_world.$set_id $arg_name = $arg_value format error", $record->{_location}) and next;
        }
      }
      else {
        $addition_args->{$arg_name} = $arg_value;
      }
    }

    #提交记录
    my $set_record = {world => $set_world
                      , id => $set_id
                      , type => $set_type
                      , version => $set_version
                      , args => $addition_args
                      , location => $set_location
                      , _location => $record->{_location}
                     };

    if (defined $set_region_id) {
      $set_record->{region} = $set_region_id;
    }

    if (defined $set_host) {
      host_add_port($self, $set_host, $set_port, "$set_world.$set_type-$set_id.router", $record->{_location});
      host_add_user($self, $set_host, $set_world, $set_user_name, $record->{_location});
    }

    if (defined $region_set_type and $region_set_type eq $set_type) {
      if (exists $record->{$region_port}) {
        push @{ $set_region->{servers} }, { ip => $set_location->{export_ip}
                                            , port => $record->{$region_port}
                                          };
      }
      else {
        $self->append_error("set $set_world.$set_id region port $region_port not configured", $record->{_location});
      }
    }

    host_add_world($self, $set_host, $world);

    $world_sets->{$set_id} = $set_record;
    $region_sets->{$set_id} = $set_record;
    push @{ $sets }, $set_record;
  }

  return 1;
}

sub host_add_port {
  (my $self, my $host, my $port, my $user, my $location) = @_;

  my $ports = $host->{ports};

  if (not exists $ports->{$port}) {
    $ports->{$port} = { user => $user, location => [ $location ] };
  }
  else {
    my $port_using = $ports->{$port};

    if ($port_using->{user} ne $user) {
      $self->append_error("device $host->{name} port $port already used by $port_using->{user}, can`t use by $user", $location);
    }
    else {
      push @{ $port_using->{location} }, $location;
    }
  }
};

sub host_add_world {
  (my $self, my $host, my $world) = @_;

  my $world_list = $host->{worlds};
  my $world_name = $world->{name};

  push @{ $world_list }, $world_name if (not grep { $_ eq $world_name } @{ $world_list } );
};

sub host_add_user {
  (my $self, my $host, my $world_name, my $user_name, my $location) = @_;

  my $user_list = $host->{users};

  if (grep { $_->{user_name} eq $user_name } @{ $user_list } ) {
    $self->append_error("device $host->{name} user $user_name already used", $location);
  }
  else {
    push @{ $user_list }, { user_name => $user_name, world_name => $world_name };
  }
};
