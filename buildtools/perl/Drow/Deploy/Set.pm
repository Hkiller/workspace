package Drow::Deploy::Set;
use strict;
use YAML qw(Dump LoadFile);
use Exporter;
use File::Basename;
use Data::Dumper;

our @ISA = qw(Exporter);
our @EXPORT = qw(generate_sets);

sub generate_sets {
  (my $output_file, my $db, my $world_name, my $set_template_dir) = @_;

  my $world = $db->world($world_name) or return;
  my $templates = load_templates($set_template_dir) or return;

  my %set_infos;
  foreach my $set_id ( keys $world->{sets} ) {
    my $set = $world->{sets}->{$set_id};
    my $set_info = generate_set($db, $world, $templates, $set);
    return if not defined $set_info;
    $set_infos{$set->{type} . "_" . $set_id} = $set_info;
  }

  if (open(my $output, '>::encoding(utf8)', $output_file)) {
    print $output Dump( \%set_infos );
    close $output;
    return 1;
  } else {
    print "generate sets: write to $output_file fail $!\n";
    return;
  }
}

sub generate_set {
  (my $db, my $world, my $templates, my $set) = @_;
  my $set_type = $set->{type};
  my $set_info = { id => $set->{id}
                   , version => $set->{version}
                   , listener => $set->{location}->{host_ip} . ":" . $set->{location}->{host_port}
                   , env => {}
                   , apps => []
                 };

  print "generate set: " . $set_type . "-" . $set->{id} . "@" . $world->{name} . ": template not exist!" and return
    if not exists $templates->{$set_type};

  #print Dumper($set);

  if (exists $world->{center}) {
    $set_info->{center} = $world->{center}->{host_ip} . ":" . $world->{center}->{port};
  }

  $set_info->{region} = $set->{region} if (exists $set->{region});

  my $template = $templates->{$set->{type}};
  my $apps = $set_info->{apps};
  my $env = $set_info->{env};

  foreach my $app ( @{$template->{apps}} ) {
    if (ref($app) eq "HASH") {
    }
    else {
      push @{ $apps }, $app;
    }
  }

  foreach my $arg_name ( keys $template->{env} ) {
    my $arg_value = $template->{env}->{$arg_name};

    if ($arg_value =~ /^\:([\w\$_-]+)$/) {
      my $dyn_name = $1;
      print "generate set: " . $set_type . "-" . $set->{id} . "@" . $world->{name} . ": $dyn_name not configured!" and return
        if not exists $set->{args}->{$dyn_name};
      my $dyn_value = $set->{args}->{$dyn_name};

      if ($dyn_name =~ qr/\-location$/ and $dyn_value =~ qr/^(\d+\.\d+\.\d+\.\d+)\:(\d+)$/) {
        my $host_ip = $1;
        my $host_port = $2;
        if ( $host_ip eq $set->{location}->{host_ip} ) {
          $dyn_value = "127.0.0.1:$host_port";
        }
      }

      $env->{$arg_name} = $dyn_value;
    }
    else {
      $env->{$arg_name} = $arg_value;
    }
  }

  return $set_info;
}

sub load_templates {
  my $set_template_dir = shift;
  my $templates = {};

  opendir(TEMPDIR, $set_template_dir) or print "can't open dir $set_template_dir: $!" and return;

  my @template_srcs = grep { $_ =~ /\.yml$/ } readdir TEMPDIR;
  close TEMPDIR;

  foreach my $template_file_name ( @template_srcs ) {
    my $template_name = basename($template_file_name, (".yml"));
    my $template_full_file_name = "$set_template_dir/$template_file_name";
    my $template_data = LoadFile($template_full_file_name) or print "Set: load template $template_full_file_name fail!" and next;
    $templates->{$template_name} = $template_data;
  }

  return $templates;
}

1;

