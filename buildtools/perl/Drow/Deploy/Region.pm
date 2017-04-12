package Drow::Deploy::Region;
use strict;
use YAML;
use Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(generate_regions);

sub generate_regions {
  (my $output_file, my $db, my $world_name) = @_;

  my $world = $db->world($world_name) or return;

  my %region_infos;
  foreach my $region_id ( keys $world->{regions} ) {
    my $region = $world->{regions}->{$region_id};
    my $region_info = { id => $region_id
                        , name => $region->{desc}
                        , type => $region->{state}
                        , servers => []
                        , 'support-chanels' => []
                        , 'support-device-categories' => []
                      };

    foreach my $server ( @{ $region->{servers} } ) {
      push @{ $region_info->{servers} }, { ip => $server->{ip}
                                           , port => $server->{port}
                                           };
    }

    foreach my $chanel ( @{ $region->{chanels} } ) {
      push @{ $region_info->{'support-chanels'} }, $chanel;
    }

    foreach my $category ( @{ $region->{'device-categories'} } ) {
      push @{ $region_info->{'support-device-categories'} }, $category;
    }

    if ( @{ $region_info->{servers} } ) {
      $region_infos{"retion_$region_id"} = $region_info;
    }
  }

  if (open(my $output, '>::encoding(utf8)', $output_file)) {
    print $output Dump( \%region_infos );
    close $output;
    return 1;
  } else {
    print "generate regions: write to $output_file fail $!\n";
    return;
  }
}

1;
