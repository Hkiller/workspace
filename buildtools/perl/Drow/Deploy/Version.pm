package Drow::Deploy::Version;
use strict;
use YAML;
use Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(generate_versions);

sub generate_versions {
  (my $output_file, my $db) = @_;

  #构造版本列表
  my @version_infos = ();
  foreach my $version_id ( keys $db->{versions} ) {
    my $version = $db->{versions}->{$version_id};
    my $version_info = { version => $version_id
                         , desc => $version->{desc}
                         , strategy => $version->{strategy}
                         , packages => []
                       };

    foreach my $package ( @{ $version->{packages} } ) {
      my $full_package = {  size => $package->{size}, md5 => $package->{md5}, url => $package->{url} };

      push @{ $version_info->{packages} }, { chanel => $package->{chanel}
                                             , 'device-category' => $package->{category}
                                             , 'full' => $full_package
                                           };
    }

    push @version_infos, $version_info;
  }

  #根据版本号排序
  @version_infos = sort {
    $a->{version} =~ qr/(\d+)\.(\d+)\.(\d+)\.(\d+)/;
    my @a_nums = ( $1, $2, $3, $4 );

    $b->{version} =~ qr/(\d+)\.(\d+)\.(\d+)\.(\d+)/;
    my @b_nums = ( $1, $2, $3, $4 );

    for(my $i = 0; $i < 4; $i++) {
      my $r = $a_nums[$i] <=> $b_nums[$i];
      return $r if $r;
    }

    return 0;
  } @version_infos;

  #输出
  if (open(my $output, '>::encoding(utf8)', $output_file)) {
    print $output Dump( \@version_infos );
    close $output;
    return 1;
  } else {
    print "generate versions: write to $output_file fail $!\n";
    return;
  }
}

1;
