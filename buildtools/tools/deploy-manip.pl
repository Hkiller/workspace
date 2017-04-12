use strict;
use File::Rsync;
use Getopt::Long;
use Data::Dumper;
use Expect;
use Drow::Deploy::DB;
use Drow::Deploy::Host;

binmode(STDOUT, ":utf8");

my $input_file_name;
my $world_name;
my $scope;
my $op;

GetOptions("input|i=s" => \$input_file_name
           , "world=s" => \$world_name
           , "scope=s" => \$scope
           , "op=s" => \$op
          );

die "input file not configured" if not defined $input_file_name;
die "scope not configured" if not defined $scope;
die "op not configured" if not defined $op;
die "op $op is unknown" if not grep { $_ eq $op } ('start', 'stop', 'restart', 'kill', 'status');

my $deploy_db = Drow::Deploy::DB->parse($input_file_name) or die "parse deploy db from $input_file_name fail!\n";
if ($deploy_db->have_error()) {
  $deploy_db->print_error();
  exit -1;
}
my $product_name = $deploy_db->{args}->{'product-name'};
my $app_name = $deploy_db->{args}->{'app-name'};

my $world = $deploy_db->world($world_name) or die "world $world_name not exist!";

foreach my $host_name ( keys $deploy_db->{hosts}) {
  my $host_info = $deploy_db->{hosts}->{$host_name};

  next if ( not grep { $_ eq $world_name } @{ $host_info->{worlds} } );

  #print Dumper($host_info);

  my $host = Drow::Deploy::Host->new($product_name, $app_name, $world_name, $host_info, $world->{repo_user});
  $host->sync_env();
  $host->print_error() and next if $host->have_error();

  #同步各个环境的版本
  if (defined $scope) {
    if ($scope eq "all" or $scope eq "center") {
      my $center_info = $world->{center};
      if ($center_info->{host_name} eq $host_name) {
        print "$op center ...";
        my $result = $host->center_execute($center_info, $op);
        print $result ? "$result\n" : "unknown\n";
      }
    }

    foreach my $set_id ( keys $world->{sets} ) {
      my $set = $world->{sets}->{$set_id};

      next if ($set->{world} ne $world_name);
      next if ($set->{location}->{host_name} ne $host_name);

      my $set_name = $set->{type} . "_" . $set->{id};

      next if $scope ne "all"
        and $scope ne $set->{type}
        and $scope ne $set->{id}
        and $scope ne $set_name;

      print "$op $set_name ...";
      my $result = $host->set_execute($set, $op);
      print $result ? "$result\n" : "unknown\n";
    }
    #print Dumper($host->{using_users});
  }
}

1;
