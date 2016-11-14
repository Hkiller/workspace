use strict;
use Getopt::Long;
use XML::Simple;
use Data::Dumper;

binmode(STDOUT, ":utf8");

my $output_file_name;
my $input_file_name;
my $version;
my $package;
my @merge_files;
my @addition_datas;

GetOptions("input|i=s" => \$input_file_name,
           "output=s" => \$output_file_name,
           "version=s" => \$version,
           "package=s" => \$package,
           "addition-data=s" => \@addition_datas,
           "merge=s" => \@merge_files);

die "version not set" if not defined $version;
die "package not set" if not defined $package;
die "input file not set" if not defined $input_file_name;
die "output file not set" if not defined $output_file_name;

sub add_permission;
sub add_activity;
sub add_receiver;
sub add_service;
sub add_data;

my $xml_in_attrs = { ForceArray => ['action', 'category'] };

my $manifest = XMLin($input_file_name, %{ $xml_in_attrs } ) or die "open input $input_file_name not exist!";
$manifest->{'android:versionName'} = $version;
$manifest->{'package'} = $package;

#print Dumper($manifest);

foreach my $data ( @addition_datas ) {
  if ($data =~ qr/^([\d\w\_\-]+)\:(.*)$/) {
    add_data($manifest, {'android:name' => $1, 'android:value' => $2 });
  }
  else {
    die "addition-data $data format error!";
  }
}

#搜索默认的启动activity
my $startup_activity;
if (exists $manifest->{'application'}) {
  my $application = $manifest->{'application'};
  if (exists $application->{activity}) {
    if (ref($application->{'activity'}) eq "ARRAY") {
      foreach my $activity ( @{ $application->{'activity'} } ) {
        if (is_activity_startup($activity)) {
          $startup_activity = $activity;
        }
      }
    } else {
      my $activity = $application->{'activity'};
      if (is_activity_startup($activity)) {
        $startup_activity = $activity;
      }
    }
  }
}

foreach my $merge_file ( @merge_files ) {
  my $merge = XMLin($merge_file, %{ $xml_in_attrs });

  if (exists $merge->{'uses-permission'}) {
    if (ref($merge->{'uses-permission'}) eq "ARRAY") {
      foreach my $permission ( @{ $merge->{'uses-permission'} } ) {
        add_permission($manifest, $permission);
      }
    } else {
      add_permission($manifest, $merge->{'uses-permission'});
    }
  }

  if (exists $merge->{'application'}) {
    my $merge_application = $merge->{'application'};

    if (exists $merge_application->{'android:allowBackup'}) {
      $manifest->{'application'}->{'android:allowBackup'} = $merge_application->{'android:allowBackup'};
    }

    if (exists $merge_application->{'android:name'}) {
      $manifest->{'application'}->{'android:name'} = $merge_application->{'android:name'};
    }

    if (exists $merge_application->{activity}) {
      if (ref($merge_application->{'activity'}) eq "ARRAY") {
        foreach my $activity ( @{ $merge_application->{'activity'} } ) {
          add_activity($manifest, $activity);
        }
      }
      else {
        add_activity($manifest, $merge_application->{'activity'});
      }
    }

    if (exists $merge_application->{receiver}) {
      if (ref($merge_application->{'receiver'}) eq "ARRAY") {
        foreach my $receiver ( @{ $merge_application->{'receiver'} } ) {
          add_receiver($manifest, $receiver);
        }
      }
      else {
        add_receiver($manifest, $merge_application->{'receiver'});
      }
    }

    if (exists $merge_application->{service}) {
      if (ref($merge_application->{'service'}) eq "ARRAY") {
        foreach my $service ( @{ $merge_application->{'service'} } ) {
          add_service($manifest, $service);
        }
      }
      else {
        add_service($manifest, $merge_application->{'service'});
      }
    }

    if (exists $merge_application->{'meta-data'}) {
      if (ref($merge_application->{'meta-data'}) eq "ARRAY") {
        foreach my $data ( @{ $merge_application->{'meta-data'} } ) {
          add_data($manifest, $data);
        }
      }
      else {
        add_data($manifest, $merge_application->{'meta-data'});
      }
    }
  }

  #print Dumper($merge);
}

sub is_activity_startup {
  my $activity = shift;

  return if not exists $activity->{'intent-filter'};
  return if not exists $activity->{'intent-filter'}->{category};

  foreach my $category ( @{ $activity->{'intent-filter'}->{category} } ) {
    return 1 if $category->{'android:name'} eq 'android.intent.category.LAUNCHER';
  }
}

sub remove_activity_startup {
  my $activity = shift;

  delete $activity->{'intent-filter'};
}

sub add_activity {
  (my $manifest, my $activity) = @_;

  my $application = $manifest->{'application'} or die "$input_file_name no application config";

  if (is_activity_startup($activity)) {
    if (defined $startup_activity) {
      remove_activity_startup( $startup_activity );

      if (exists $activity->{'meta-data'}) {
        if (ref($activity->{'meta-data'}) eq "ARRAY") {
          foreach my $data ( @{ $activity->{'meta-data'} } ) {
            if ($data->{'android:value'} eq '$pre-activity') {
              $data->{'android:value'} = $startup_activity->{'android:name'};
            }
          }
        } else {
          my $data = $activity->{'meta-data'};
          if ($data->{'android:value'} eq '$pre-activity') {
            $data->{'android:value'} = $startup_activity->{'android:name'};
          }
        }
      }
    }

    $startup_activity = $activity;
  }

  if (not exists $application->{activity}) {
    $application->{activity} = $activity;
  } elsif (ref($application->{activity}) eq "ARRAY") {
    push @{ $application->{activity} }, $activity;
  } else {
    $application->{activity} = [ $application->{activity}, $activity ];
  }

  return 1;
}

sub add_receiver {
  (my $manifest, my $receiver) = @_;

  my $application = $manifest->{'application'} or die "$input_file_name no application config";

  if (not exists $application->{receiver}) {
    $application->{receiver} = $receiver;
  } elsif (ref($application->{receiver}) eq "ARRAY") {
    push @{ $application->{receiver} }, $receiver;
  } else {
    $application->{receiver} = [ $application->{receiver}, $receiver ];
  }

  return 1;
}

sub add_permission {
  (my $manifest, my $permission) = @_;

  if (not exists $manifest->{'uses-permission'}) {
    $manifest->{'uses-permission'} = $permission;
  } elsif (ref($manifest->{'uses-permission'}) eq "ARRAY") {
    push @{ $manifest->{'uses-permission'} }, $permission;
  } else {
    $manifest->{'uses-permission'} = [ $manifest->{'uses-permission'}, $permission ];
  }

  return 1;
}

sub add_service {
  (my $manifest, my $service) = @_;

  my $application = $manifest->{'application'} or die "$input_file_name no application config";

  if (not exists $application->{service}) {
    $application->{service} = $service;
  } elsif (ref($application->{service}) eq "ARRAY") {
    push @{ $application->{service} }, $service;
  } else {
    $application->{service} = [ $application->{service}, $service ];
  }

  return 1;
}

sub add_data {
  (my $manifest, my $data) = @_;

  my $application = $manifest->{'application'} or die "$input_file_name no application config";

  if (not exists $application->{'meta-data'}) {
    $application->{'meta-data'} = $data;
  } elsif (ref($application->{'meta-data'}) eq "ARRAY") {
    push @{ $application->{'meta-data'} }, $data;
  } else {
    $application->{'meta-data'} = [ $application->{'meta-data'}, $data ];
  }

  return 1;
}

XMLout($manifest,
       RootName => "manifest"
       , outputfile => $output_file_name
       , XMLDecl => '<?xml version="1.0" encoding="utf-8"?>'
      );

1;
