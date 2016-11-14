use strict;
use Getopt::Long;
use Data::Dumper;
use Drow::Deploy::DB;
use Drow::Deploy::Region;
use Drow::Deploy::Version;
use Drow::Deploy::Set;

binmode(STDOUT, ":utf8");

my $input_file_name;
my $regions_file_name;
my $versions_file_name;
my $sets_file_name;
my $set_template_dir;
my $summary_dir_name;
my $world;

GetOptions("input|i=s" => \$input_file_name
           , "world=s" => \$world
           , "gen-regions=s" => \$regions_file_name
           , "gen-versions=s" => \$versions_file_name
           , "gen-sets=s" => \$sets_file_name
           , "set-template=s" => \$set_template_dir
           , "gen-summary=s" => \$summary_dir_name
          );

my $deploy_db = Drow::Deploy::DB->parse($input_file_name) or die "parse deploy db from $input_file_name fail!\n";
if ($deploy_db->have_error()) {
  $deploy_db->print_error();
  exit -1;
}

#print Dumper( $deploy_db->{worlds} );

if ($regions_file_name) {
  die "gen-regions: word not set!" if not $world;
  generate_regions($regions_file_name, $deploy_db, $world);
}

if ($versions_file_name) {
  generate_versions($versions_file_name, $deploy_db);
}

if ($sets_file_name) {
  die "gen-sets: word not set!" if not $world;
  die "gen-sets: set-template not configured!" if not $set_template_dir;
  generate_sets($sets_file_name, $deploy_db, $world, $set_template_dir);
}

if ($summary_dir_name) {
  $deploy_db->generate_summary($summary_dir_name);
}

1;

