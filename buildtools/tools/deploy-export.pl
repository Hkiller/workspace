use strict;
use Getopt::Long;
use Data::Dumper;
use Expect;
use Drow::Deploy::DB;
use Drow::Deploy::Host;

binmode(STDOUT, ":utf8");

my $input_file_name;
my $world_name;
my $version;
my $version_dir;
my $scope;

GetOptions("input|i=s" => \$input_file_name
           , "world=s" => \$world_name
           , "version=s" => \$version
           , "version-dir=s" => \$version_dir
           , "scope=s" => \$scope
          );

die "input file not configured" if not defined $input_file_name;
die "version $version src dir configured" if defined $version and not defined $version_dir;

my $deploy_db = Drow::Deploy::DB->parse($input_file_name) or die "parse deploy db from $input_file_name fail!\n";
if ($deploy_db->have_error()) {
  $deploy_db->print_error();
  exit -1;
}
my $product_name = $deploy_db->{args}->{'product-name'};
my $app_name = $deploy_db->{args}->{'app-name'};

my $world = $deploy_db->world($world_name) or die "world $world_name not exist!";

#print Dumper($world);

foreach my $host_name ( keys $deploy_db->{hosts}) {
  my $host_info = $deploy_db->{hosts}->{$host_name};

  next if ( not grep { $_ eq $world_name } @{ $host_info->{worlds} } );

  #print Dumper($host_info);

  my $host = Drow::Deploy::Host->new($product_name, $app_name, $world_name, $host_info, $world->{repo_user});
  $host->sync_env();
  $host->print_error() and next if $host->have_error();

  #同步版本
  $host->export_version($version, $version_dir) if (defined $version);

  #同步各个环境的版本
  if (defined $scope) {
    if ($scope eq "all" or $scope eq "center") {
      my $center_info = $world->{center};
      if ($center_info->{host_name} eq $host_name) {
        #print Dumper($center_info);
        $host->sync_env_sync_user($center_info->{user}, $center_info->{version}, "center");
      }
    }

    foreach my $set_id ( keys $world->{sets} ) {
      my $set = $world->{sets}->{$set_id};

      next if ($set->{world} ne $world_name);
      next if ($set->{location}->{host_name} ne $host_name);
      next if $scope ne "all" and $scope ne $set->{type} and $scope ne $set->{id};

      $host->sync_env_sync_user($set->{location}->{user}, $set->{version}, $set->{type} . "_" . $set->{id});
    }
    #print Dumper($host->{using_users});
  }

  $host->print_error() and next if $host->have_error();
}

1;
