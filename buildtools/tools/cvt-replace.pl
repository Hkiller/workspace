use strict;
use Encode;
use Getopt::Long;

binmode(STDOUT, ":utf8");

my $inputFile;
my $outputFile;
my @configFiles;

GetOptions("input=s" => \$inputFile
           , "output=s" => \$outputFile
           , "config=s" => \@configFiles
           );

$inputFile = decode("utf8", $inputFile);
$outputFile = decode("utf8", $outputFile);

my %replaceList;
foreach my $configFile (@configFiles) {
  $configFile = decode("utf8", $configFile);
  open(my $config, "<::encoding(utf8)", $configFile) or die "open config file $configFile fail $!";

  while (my $line = <$config>) {
    next if $line !~ /^\s*([\w\d:_-]+)=\s*(.*)\s*$/;

    $replaceList{$1} = $2;
  }

  close($config);
}

open(my $input, "<::encoding(utf8)", $inputFile) or die "open input file $inputFile fail $!";
open(my $output, ">::encoding(utf8)", $outputFile) or die "open output file $outputFile fail $!";

while (my $line = <$input>) {
  foreach my $from (keys %replaceList) {
    $line =~ s/\$\{$from\}/$replaceList{$from}/;
  }

  print $output "$line";
}

1;
