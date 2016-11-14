use strict;
use Encode;
use XML::Simple;
use Getopt::Long;
use Data::Dumper;

binmode(STDOUT, ":utf8");

my $inputFile;
my $outputH;
my $outputC;
my $prefix;
my $metalib;
my @meta_headers;

GetOptions("input-file=s" => \$inputFile,
           "output-h=s"   => \$outputH,
           "output-c=s"   => \$outputC,
           "prefix=s" => \$prefix,
           "metalib=s" => \$metalib,
           "meta-h=s" => \@meta_headers);

$inputFile = decode("utf8", $inputFile);
$outputH = decode("utf8", $outputH);
$outputC = decode("utf8", $outputC);

sub entry_is_auto_gen {
  my $v = shift;

  return 0 if not defined $v;
  return 1 if $v =~ m/^-.*/;
  return 1 if $v =~ m/^\@.+\|.*/;
  return 0;
}

my $type_info_def = { int8 => { formator => "\"FMT_INT8_T\"", type => "int8_t" }
               , uint8 => { formator => "\"FMT_UINT8_T\"", type => "uint8_t" }
               , int16 => { formator => "\"FMT_INT16_T\"", type => "int16_t" }
               , uint16 => { formator => "\"FMT_UINT16_T\"", type => "uint16_t" }
               , int32 => { formator => "\"FMT_INT32_T\"", type => "int32_t" }
               , uint32 => { formator => "\"FMT_UINT32_T\"", type => "uint32_t" }
               , int64 => { formator => "\"FMT_INT64_T\"", type => "int64_t" }
               , uint64 => { formator => "\"FMT_UINT64_T\"", type => "uint64_t" }
               , string => { formator => "\%s", type => "const char *" }
               , datetime => { formator => "\%s", type => "time_t"}
};

my $xml_lib_source = XMLin($inputFile, KeyAttr => {struct => 'name', union => 'name'}, ForceArray => [ 'struct', 'union' ]);

sub generate_h_entry {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  return if entry_is_auto_gen($entry->{desc});

  my $type_info = $type_info_def->{$entry->{type}};

  print $meta_name . "." . $entry_name . ": type '" . $entry->{type} ."' not support!\n" and return
    if not defined $type_info;

  print $output "\n    , $type_info->{type} $entry_name";
}

sub generate_c_entry_formator {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  if ($entry->{type} eq "string") {
    print $output "    strncpy(data." . $entry_name . ", " . $entry_name . ", sizeof(data." . $entry_name . "));\n";
  }
  else {
    print $output "    data." . $entry_name . " = " . $entry_name . ";\n";
  }
}

sub generate_c_entry_arg {
  (my $meta_name, my $meta, my $entry_name, my $entry, my $output) = @_;

  if (defined $entry->{desc} and $entry->{desc} =~ m/^-.*/) {
  }
  elsif (defined $entry->{desc} and $entry->{desc} =~ m/^\@(.+)\|.*/) {
    my $ref_arg = $1;
    my $type_info = $type_info_def->{$entry->{type}};

    return if not defined $type_info;

    if (exists $type_info->{"to-string-init"}) {
      print $output "        , ($type_info->{type})buf_" . $entry_name . "\n";
    }
    else {
      print $output "        , ($type_info->{type})" . $ref_arg. "\n";
    }
  }
  else {
    my $type_info = $type_info_def->{$entry->{type}};

    return if not defined $type_info;

    if (exists $type_info->{"to-string-init"}) {
      print $output "        , ($type_info->{type})buf_" . $entry_name . "\n";
    }
    else {
      print $output "        , ($type_info->{type})" . $entry_name. "\n";
    }
  }
}

sub foreach_entry {
  (my $meta_name, my $output, my $fun) = @_;

  my $meta = $xml_lib_source->{struct}->{$meta_name};

  if (ref($meta->{entry}) eq "ARRAY") {
    foreach my $entry (@{$meta->{entry}}) {
      $fun->($meta_name, $meta, $entry->{name}, $entry, $output);
    }
  }
  elsif (ref($meta->{entry}) eq "HASH") {
    my $entry = $meta->{entry};
    $fun->($meta_name, $meta, $entry->{name}, $entry, $output);
  }
}

sub generate_h {
  my $output = shift;

  print $output "#ifndef _DBLOG_". $prefix . "_H_INCLEDE_\n";
  print $output "#define _DBLOG_" . $prefix . "_H_INCLEDE_\n";
  print $output "#include \"cpe/pal/pal_time.h\"\n";
  print $output "#include \"gd/app/app_types.h\"\n";
  print $output "\n";

  print $output "#ifdef __cplusplus\n";
  print $output "extern \"C\" {\n";
  print $output "#endif\n";

  if (exists $xml_lib_source->{struct}) {
    foreach my $meta_name ( keys %{$xml_lib_source->{struct}} ) {
      my $meta = $xml_lib_source->{struct}->{$meta_name};

      print $output "\nvoid log_" . $prefix . "_" . $meta_name . "(gd_app_context_t app";

      foreach_entry($meta_name, $output, \&generate_h_entry);

      print $output ");\n";
    }
  }

  print $output "\n";
  print $output "#ifdef __cplusplus\n";
  print $output "}\n";
  print $output "#endif\n";
  print $output "\n";
  print $output "#endif\n";
}

sub generate_c {
  (my $output) = @_;

  print $output "#include <assert.h>\n";
  print $output "#include \"cpe/pal/pal_stdio.h\"\n";
  print $output "#include \"cpe/pal/pal_time.h\"\n";
  print $output "#include \"cpe/dr/dr_metalib_manage.h\"\n";
  print $output "#include \"gd/app/app_context.h\"\n";
  print $output "#include \"svr/dblog/agent/dblog_agent.h\"\n";
  foreach my $meta_header ( @meta_headers ) {
    print $output "#include \"" . $meta_header . "\"\n";
  }
  print $output "\n";

  print $output "extern char " . $metalib . "[];\n";
  print $output "\n";

  if (exists $xml_lib_source->{struct}) {
    foreach my $meta_name ( keys %{$xml_lib_source->{struct}} ) {
      print $output "\nvoid log_" . $prefix . "_" . $meta_name . "(gd_app_context_t app";
      foreach_entry($meta_name, $output, \&generate_h_entry);
      print $output ")\n";
      print $output "{\n";

      print $output "    static LPDRMETA data_meta = NULL;\n";
      print $output "    dblog_agent_t dblog;\n";
      print $output "    " . uc($meta_name) . " data;\n";
      print $output "    \n";
      print $output "    if (data_meta == NULL) {\n";
      print $output "        data_meta = dr_lib_find_meta_by_name((LPDRMETALIB)" . $metalib . ", \"" . $meta_name . "\");\n";
      print $output "        assert(data_meta);\n";
      print $output "    }\n";
      print $output "    \n";
      print $output "    dblog= dblog_agent_find_nc(app, NULL);\n";
      print $output "    if (dblog == NULL) return;\n";
      print $output "    \n";

      foreach_entry($meta_name, $output, \&generate_c_entry_formator);

      print $output "    \n";
      print $output "    dblog_agent_log(dblog, &data, sizeof(data), data_meta);\n";

      print $output "}\n";
    }
  }
}


open(my $output_h, '>::encoding(utf8)', $outputH) or die "open output head file $outputH fail $!";
generate_h($output_h);
close($output_h);

open(my $output_c, '>::encoding(utf8)', $outputC) or die "open output file $outputH fail $!";
generate_c($output_c);
close($output_c);

