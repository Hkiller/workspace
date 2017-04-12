use strict;
use Getopt::Long;

binmode(STDOUT, ":utf8");

my $output_file_name;
my @input_files;
my $compiler;

GetOptions("input|i=s" => \@input_files,
           "output=s" => \$output_file_name,
           "compiler=s" => \$compiler);

my @symbols = ();

foreach my $input_file ( @input_files ) {
  open(my $input, $input_file) or die "open input file $input_file fail, $!";

  while (my $line = <$input> ) {
    next if $line =~ /^\s*$/;

    if ($line =~ /^(.*)\r?\n$/) {
      $line = $1;
    }

    push @symbols, $line;
  }

  close $input;
}

open(my $output, '>::encoding(utf8)', $output_file_name) or die "open output file $output_file_name fail $!";

if ($output_file_name =~ /\.c$/) {
  print $output "#include \"cpe/pal/pal_stdio.h\"\n";
  print $output "#include \"gd/app/app_library.h\"\n";
  print $output "#include \"gd/app/app_log.h\"\n";
  print $output "\n";
  foreach my $symbol ( @symbols ) {
    print $output "extern char " . $symbol . "[];\n";
  }
  print $output "\n";

  print $output "int init_symbols(void) {\n";
  print $output "    int rv = 0;\n";
  print $output "    if (gd_app_symbols_init() != 0) {\n";
  print $output "        printf(\"init_symbols: app symbols init fail\\n\");\n";
  print $output "        return -1;\n";
  print $output "    }\n";
  foreach my $symbol ( @symbols ) {
    print $output "    if (gd_app_lib_register_symbol(\"\", \"" . $symbol . "\", " . $symbol . ") != 0) {\n";
    print $output "        printf(\"init_symbols: regist symbol " . $symbol . " fail\\n\");\n";
    print $output "        rv = -1;\n";
    print $output "    }\n";
  }
  print $output "    return rv;\n";
  print $output "}\n";
}
else {
  if ($compiler eq "vc") {
    print $output "EXPORTS\n";
  }

  foreach my $symbol ( @symbols ) {
    if ($compiler eq "vc" or $compiler eq "gcc") {
      print $output $symbol . "\n";
    }
    else {
      print $output "_$symbol\n";
    }
  }
}

close $output;

1;
