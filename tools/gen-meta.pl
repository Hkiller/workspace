use strict;
use YAML;
use Encode;
use POSIX;
use XML::Simple;
use Getopt::Long;
use Spreadsheet::ParseExcel;
use Data::Dumper;
use File::Basename;
use File::Spec;
use File::stat;

binmode(STDOUT, ":utf8");

my $inputFile;
my $inputSheet;
my $outputFile;
my @metaLibDirs;
my $metaLibFile;
my $metaName;
my $startLine;
my $chanel;

GetOptions("input-file=s" => \$inputFile,
           "input-sheet=s" => \$inputSheet,
           "output=s"   => \$outputFile,
           "meta-lib-dirs=s" => \@metaLibDirs,
           "meta-lib=s" => \$metaLibFile,
           "meta-name=s" => \$metaName,
           "start-line=i" => \$startLine,
           "chanel=s" => \$chanel);

$startLine = 1 if ! defined $startLine;

$inputSheet = decode("utf8", $inputSheet);
$inputFile = decode("utf8", $inputFile);

my %macrosgroups;
my %macros;
my %metaLib;

push @metaLibDirs, dirname($metaLibFile);

sub load_meta_lib {
   my $file = shift;

   my $xml_lib_source =
     XMLin($file,
           KeyAttr => {struct => 'name', union => 'name', macrosgroup => 'name', macro => 'name' },
           ForceArray => [ 'struct', 'union', 'macrosgroup', 'macro', 'include' ]);

   if (exists $xml_lib_source->{include}) {
     PROCESS_INCLUDE: foreach my $include ( @{ $xml_lib_source->{include} } ) {
       foreach my $search_dir ( @metaLibDirs ) {
         my $include_full = File::Spec->catfile($search_dir, $include->{file});
         next if not stat($include_full);
         load_meta_lib($include_full);
         next PROCESS_INCLUDE;
       }
     }
   }

   if (exists $xml_lib_source->{macrosgroup}) {
     foreach my $macrogroup_name ( keys %{$xml_lib_source->{macrosgroup}} ) {
       $macrosgroups{$macrogroup_name} = $xml_lib_source->{macrosgroup}->{$macrogroup_name};
     }
   }

   if (exists $xml_lib_source->{macro}) {
     foreach my $macro_name ( keys %{$xml_lib_source->{macro}} ) {
       $macros{$macro_name} = $xml_lib_source->{macro}->{$macro_name}->{value};
     }
   }

   if (exists $xml_lib_source->{struct}) {
     foreach my $meta_name ( keys %{$xml_lib_source->{struct}} ) {
       $metaLib{$meta_name} = $xml_lib_source->{struct}->{$meta_name};
       $metaLib{$meta_name}->{meta_type} = 'struct';
     }
   }

   if (exists $xml_lib_source->{union}) {
     foreach my $meta_name ( keys %{$xml_lib_source->{union}} ) {
       $metaLib{$meta_name} = $xml_lib_source->{union}->{$meta_name};
       $metaLib{$meta_name}->{meta_type} = 'union';
     }
   }
}

load_meta_lib($metaLibFile);
#print Dump(\%metaLib);

my %input_col_processors;

sub is_entry_basic_type {
  my $entry = shift;

  return $entry->{type} =~ /^(string)|(char)|(float)|(double)|(u?int(8|16|32|64))$/;
}

sub add_col_processor {
  my ($colName, $fun) = @_;

  $input_col_processors{$colName} = []
    if not exists $input_col_processors{$colName};

  push @{$input_col_processors{$colName}}, $fun;
}

sub calc_col_fun {
  my ($colName, $resultColName, $selector, $entry) = @_;

  if ($selector && $selector =~ /split\s*\(\s*'([^']+)'\s*,\s*(\d+)\s*\)/) {
    my $sep = $1;
    my $getPos = $2;

    return sub {
      my ($row, $value, $input_row) = @_;
      my @values = split(/$sep/, $value);
      if ($getPos < @values + 0) {
        $row->{$resultColName} = $values[$getPos] if ($values[$getPos] !~ m/^\s*$/);
      }
    };
  }

  if ($selector && $selector =~ /match\s*\(\s*'([^']+)'\s*\)/) {
    my $matcher = $1;
    return sub {
      my ($row, $value, $input_row) = @_;
      if ($value =~ m/$matcher/) {
        $row->{$resultColName} = $1 if ($1 !~ m/^\s*$/) ;
      }
    };
  }

  if ($selector && $selector =~ /parse-time\s*\(\s*'([^']+)'\s*\)/) {
    my $matcher = $1;
    return sub {
      my ($row, $value, $input_row) = @_;
      if ($value =~ m/$matcher/) {
        if ($1 !~ m/^\s*$/) {
          my $input_time = $1;

          if ($input_time =~ /(\d+)-(\d+)-(\d+) (\d\d):(\d\d):(\d\d)/) {
            $row->{$resultColName} = mktime($6, $5, $4, $3, $2 - 1, $1 - 1900, 0, 0);
          }
          else {
            die("$colName time [$input_time] format error\n");
          }
        }
      }
    };
  }

  if ($selector && $selector =~ /macro-to-value\s*\(\s*'([^']+)'\s*\)/) {
    my $matcher = $1;

    my $macrogroup_name = $entry->{bindmacrosgroup};
    print "bindmacrosgroup not defined!\n" and return sub {} if not defined $macrogroup_name;

    my $macrogroup = $macrosgroups{$macrogroup_name};
    print "macrosgroup $macrogroup_name not exist!\n" and return sub{} if not defined $macrogroup;

    return sub {
      my ($row, $value, $input_row) = @_;
      if ($value =~ /$matcher/) {
        my $orig = $1;

        if ($macrogroup) {
          foreach my $macro_name ( keys %{ $macrogroup->{macro} } ) {
            my $macro_cname = $macrogroup->{macro}->{$macro_name}->{cname} || "";
            my $macro_value = $macrogroup->{macro}->{$macro_name}->{value};

            if ($macro_cname eq $orig or $macro_name eq $orig) {
              $row->{$resultColName} = $macro_value;
              return;
            }
          }
        }

        $row->{$resultColName} = $orig if ( $orig !~ m/^\s*$/) ;
      }
    };
  }

  if ($selector && $selector =~ /convert\s*\(\s*'([^']+)'\s*:\s*([^)]*)\)/) {
    my $matcher = $1;

    my %converts;
    my $default;

    foreach my $selectItem (split(/,/, $2)) {
      if ($selectItem =~ /^\s*'([^']+)'\s*\?\s*([^\s]+)\s*$/) {
        $converts{$1} = $2;
      }
      elsif ( $selectItem =~ /^\s*default\s*([^\s]+)\s*$/ ) {
        $default = $1;
      }
    }

    return sub {
      my ($row, $value, $input_row) = @_;

      if ($value =~ /$matcher/) {
        if (not defined $1) {
          if ($default) {
              $row->{$resultColName} = $default;
          }
        }
        else {
          my $input = $1;
          if (defined $input && exists $converts{$input} ) {
            $row->{$resultColName} = $converts{$input};
          } elsif ($default) {
            if ($default eq '$1') {
              $row->{$resultColName} = $value if ( $value !~ m/^\s*$/ );
            }
            else {
              $row->{$resultColName} = $default;
            }
          }
        }
      }
    };
  }

  if ($selector && $selector =~ /replace\s*\(\s*'([^']+)'\s*:\s*'([^']+)'\s*\)/) {
    my $replace_from = $1;
    my $replace_to = $2;

    return sub {
      my ($row, $value, $input) = @_;
      $value =~ s/$replace_from/$replace_to/;
      $row->{$resultColName} = $value if ($value !~ m/^\s*$/ );
    };
  }

  if ($selector && $selector =~ /value\s*\(\s*'([^']+)'\s*\)/) {
    my $value = $1;

    return sub {
      my ($row, $old_value) = @_;
      $row->{$resultColName} = $value if ($value !~ m/^\s*$/);
    };
  }

  return sub {
    my ($row, $value, $input_row) = @_;
    $row->{$resultColName} = $value if ($value !~ m/^\s*$/);
  };
}

sub analize_entry_processor_union {
  my ($meta, $entry, $cname_pre_fix, $cname_post_fix, $col_fun_derator) = @_;

  return if (not exists $entry->{customattr});

  my $input_col_name;

  if (exists $entry->{cname}) {
    $input_col_name = $cname_pre_fix . $entry->{cname} . $cname_post_fix;
  }

  if ($entry->{customattr} =~ /match\s*\(\s*'([^']*)'\s*\)/) {
    my $filter = $1;

    my $subMeta = $metaLib{$entry->{type}};
    if (not $subMeta) {
      print("$entry->{name} ref type $entry->{type} is unknown!\n");
      return;
    }

    analize_submeta($subMeta,
                    $cname_pre_fix,
                    $cname_post_fix,
                    $entry,
                    sub {
                      my $innerSub = shift;

                      my $newSub = sub {
                        my ($row, $value, $input_row) = @_;

                        if (defined $input_col_name) {
                          return if not exists $input_row->{$input_col_name};
                          return if $input_row->{$input_col_name} !~ $filter;
                        }
                        else {
                          return if $value !~ $filter;
                        }

                        $innerSub->($row, $value, $input_row);
                      };

                      $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                      return $newSub;
                    });
  }
  else {
    printf "aaa\n";
  }
}

sub analize_entry_processor_struct_seq_basic {
  my ($meta, $entry, $cname_pre_fix, $cname_post_fix, $count, $col_fun_derator) = @_;

  return if not exists $entry->{cname};

  my $input_col_name = $cname_pre_fix . $entry->{cname} . $cname_post_fix;

  if ($entry->{customattr} && $entry->{customattr} =~ /split\s*\(\s*'([^']+)'\s*\)/) {
    my $sep = $1;
    my $getPos = $2;

    my $col_fun = sub {
      my ($row, $value, $input_row) = @_;

      my @value_list = split(/$sep/, $value);

      if (@value_list) {
        $row->{$entry->{name}} = [];

        @{$row->{$entry->{name}}} = @value_list;
      }
    };

    $col_fun = $col_fun_derator->($col_fun) if defined $col_fun_derator;
    add_col_processor($input_col_name, $col_fun);
  } elsif ($entry->{customattr} && $entry->{customattr} =~ /make_array\s*\(\s*'([^']+)'\s*\)/) {
    my @postfixs = split(':', $1);

    foreach my $pos ( 0 .. $#postfixs ) {
      my $col_name = $input_col_name . $postfixs[$pos];

      my $col_fun = sub {
        my ($row, $value, $input_row) = @_;

        return if (not defined $value) or ($value eq "");

        if ($value !~ m/^\s*$/) {
          $row->{$entry->{name}} = []
            if not exists $row->{$entry->{name}};

          while ( @{$row->{$entry->{name}}} < $pos) {
            push @{$row->{$entry->{name}}}, "";
          }

          ${$row->{$entry->{name}}}[$pos] = $value;
        }
      };

      $col_fun = $col_fun_derator->($col_fun) if defined $col_fun_derator;
      add_col_processor($col_name, $col_fun);
    }
  } elsif ($entry->{customattr} && $entry->{customattr} =~ /make_array_prefix\s*\(\s*'([^']+)'\s*\)/) {
    my @prefixs = split(':', $1);

    foreach my $pos ( 0 .. $#prefixs ) {
      my $col_name = $prefixs[$pos] . $input_col_name;

      my $col_fun = sub {
        my ($row, $value, $input_row) = @_;

        return if (not defined $value) or ($value eq "");

        if ($value !~ m/^\s*$/) {
          $row->{$entry->{name}} = []
            if not exists $row->{$entry->{name}};

          while ( @{$row->{$entry->{name}}} < $pos) {
            push @{$row->{$entry->{name}}}, "";
          }

          ${$row->{$entry->{name}}}[$pos] = $value;
        }
      };

      $col_fun = $col_fun_derator->($col_fun) if defined $col_fun_derator;
      add_col_processor($col_name, $col_fun);
    }
  }
}

sub analize_entry_processor_struct_seq_compose {
  my ($meta, $entry, $cname_pre_fix, $cname_post_fix, $count, $col_fun_derator) = @_;

  my $subMeta = $metaLib{$entry->{type}};
  if (not $subMeta) {
    print("$entry->{name} ref type $entry->{type} is unknown!\n");
    return;
  }

  if ($subMeta->{meta_type} eq "union") {
    analize_submeta($subMeta, $cname_pre_fix, $cname_post_fix, $entry, $col_fun_derator);
    return;
  }

  return if not exists $entry->{customattr};

  if ($entry->{customattr} eq "repeat") {
    foreach my $c ( 1 .. $count ) {
      analize_meta_processors($subMeta,
                              $cname_pre_fix,
                              "$c$cname_post_fix",
                              sub {
                                my $innerSub = shift;

                                my $newSub = sub {
                                  my ($row, $value, $input_row) = @_;

                                  return if (not $value) or ($value eq "");

                                  $row->{$entry->{name}} = []
                                    if not exists $row->{$entry->{name}};

                                  while ( @{$row->{$entry->{name}}} < $c) {
                                    push @{$row->{$entry->{name}}}, {};
                                  }

                                  $innerSub->(${$row->{$entry->{name}}}[$c - 1], $value, $input_row);
                                };

                                $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                                return $newSub;
                              });
    }
  } elsif ($entry->{customattr} =~ /split\s*\(\s*'([^']+)'\s*\)/) {
    my $sep = $1;
    analize_meta_processors($subMeta,
                            $cname_pre_fix,
                            $cname_post_fix,
                            sub {
                              my $innerSub = shift;

                              my $newSub = sub {
                                my ($row, $value, $input_row) = @_;
                                my @values = split /$sep/, $value;
                                return if not @values;

                                $row->{$entry->{name}} = []
                                  if not exists $row->{$entry->{name}};

                                my $seq = $row->{$entry->{name}};

                                foreach my $pos (0 .. $#values) {
                                  push @{$seq}, {} if $pos > $#{$seq};
                                  $innerSub->(${$seq}[$pos], $values[$pos], $input_row);
                                }
                              };

                              $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                              return $newSub;
                            });
  }
  elsif ($entry->{customattr} && $entry->{customattr} =~ /make_array\s*\(\s*'([^']+)'\s*\)/) {
    my @postfixs = split(':', $1);

    foreach my $pos ( 0 .. $#postfixs ) {
      analize_meta_processors($subMeta,
                              $cname_pre_fix,
                              "$postfixs[$pos]$cname_post_fix",
                              sub {
                                my $innerSub = shift;

                                my $newSub = sub {
                                  my ($row, $value, $input_row) = @_;

                                  return if (not $value) or ($value eq "");

                                  $row->{$entry->{name}} = []
                                    if not exists $row->{$entry->{name}};

                                  while ( @{$row->{$entry->{name}}} <= $pos) {
                                    push @{$row->{$entry->{name}}}, {};
                                  }

                                  $innerSub->(${$row->{$entry->{name}}}[$pos], $value, $input_row);
                                };

                                $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                                return $newSub;
                              });
    }
  }
  elsif ($entry->{customattr} && $entry->{customattr} =~ /make_array_prefix\s*\(\s*'([^']+)'\s*\)/) {
    my @prefixs = split(':', $1);

    foreach my $pos ( 0 .. $#prefixs ) {
      analize_meta_processors($subMeta,
                              $cname_pre_fix . $prefixs[$pos],
                              $cname_post_fix,
                              sub {
                                my $innerSub = shift;

                                my $newSub = sub {
                                  my ($row, $value, $input_row) = @_;

                                  return if (not $value) or ($value eq "");

                                  $row->{$entry->{name}} = []
                                    if not exists $row->{$entry->{name}};

                                  while ( @{$row->{$entry->{name}}} <= $pos) {
                                    push @{$row->{$entry->{name}}}, {};
                                  }

                                  $innerSub->(${$row->{$entry->{name}}}[$pos], $value, $input_row);
                                };

                                $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                                return $newSub;
                              });
    }
  }
  else {
    print("$entry->{name} seq-type $entry->{customattr} is unknown!\n");
  }
}

sub analize_entry_processor_struct {
  my ($meta, $entry, $cname_pre_fix, $cname_post_fix, $col_fun_derator) = @_;

  my $entry_count = $entry->{count};
  if (defined $entry_count and exists $macros{$entry_count}) {
    $entry_count = $macros{$entry_count};
  }

  if (defined $entry->{count} and $entry_count != 1) {
    if (is_entry_basic_type($entry)) {
      return analize_entry_processor_struct_seq_basic($meta, $entry, $cname_pre_fix, $cname_post_fix, $entry_count, $col_fun_derator);
    }
    else {
      return analize_entry_processor_struct_seq_compose($meta, $entry, $cname_pre_fix, $cname_post_fix, $entry_count, $col_fun_derator);
    }
  }
  else {
    if (is_entry_basic_type($entry)) {
      return if not exists $entry->{cname};

      my $input_col_name = $cname_pre_fix . $entry->{cname} . $cname_post_fix;

      my $col_fun = calc_col_fun($entry->{cname}, $entry->{name}, $entry->{customattr}, $entry);

      $col_fun = $col_fun_derator->($col_fun)
        if defined $col_fun_derator;

      add_col_processor($input_col_name, $col_fun);
      return;
    }
    else {
      my $subMeta = $metaLib{$entry->{type}};
      if (not $subMeta) {
        print("$entry->{name} ref type $entry->{type} is unknown!\n");
        return;
      }

      if ($entry->{customattr} && $entry->{customattr} =~ /make_postfix\s*\(\s*'([^']+)'\s*\)/) {
        my $postfix = $1;
        analize_submeta($subMeta, ( $entry->{cname} || "" ) . $cname_pre_fix, $postfix.$cname_post_fix, $entry, $col_fun_derator);
      }
      elsif ($entry->{customattr} && $entry->{customattr} =~ /make_prefix\s*\(\s*'([^']+)'\s*\)/) {
        my $prefix = $1;
        analize_submeta($subMeta, ( $entry->{cname} || "" ) . $cname_pre_fix . $prefix, $cname_post_fix, $entry, $col_fun_derator);
      }
      else {
        analize_submeta($subMeta, ( $entry->{cname} || "" ) . $cname_pre_fix, $cname_post_fix, $entry, $col_fun_derator);
      }
    }
  }
}

sub analize_submeta {
  my ($subMeta, $cname_pre_fix, $cname_post_fix, $entry, $col_fun_derator) = @_;

    analize_meta_processors($subMeta,
                            $cname_pre_fix,
                            $cname_post_fix,
                            sub {
                              my $innerSub = shift;

                              my $newSub = sub {
                                my ($row, $value, $input_row) = @_;

                                if (not exists $row->{$entry->{name}} ){
                                  my $subRow = {};
                                  $innerSub->($subRow, $value, $input_row);
                                  if (keys %{$subRow}) {
                                    $row->{$entry->{name}} = $subRow;
                                  }
                                }
                                else {
                                  $innerSub->($row->{$entry->{name}}, $value, $input_row);
                                }
                              };

                              $newSub = $col_fun_derator->($newSub) if $col_fun_derator;

                              return $newSub;
                            });
}

sub analize_meta_processors {
  my ($meta, $cname_pre_fix, $cname_post_fix, $col_fun_derator) = @_;

  my $type = $meta->{meta_type};

  if ($type eq "struct") {
    if (ref($meta->{entry}) eq "ARRAY") {
      foreach my $entry (@{$meta->{entry}}) {
        analize_entry_processor_struct($meta, $entry, $cname_pre_fix, $cname_post_fix, $col_fun_derator);
      }
    }
    elsif (ref($meta->{entry}) eq "HASH") {
      my $entry = $meta->{entry};
      analize_entry_processor_struct($meta, $entry, $cname_pre_fix, $cname_post_fix, $col_fun_derator);
    }
  }
  else {
    if (ref($meta->{entry}) eq "ARRAY") {
      foreach my $entry (@{$meta->{entry}}) {
        analize_entry_processor_union($meta, $entry, $cname_pre_fix, $cname_post_fix, $col_fun_derator);
      }
    }
    elsif (ref($meta->{entry}) eq "HASH") {
      my $entry = $meta->{entry};
      analize_entry_processor_union($meta, $entry, $cname_pre_fix, $cname_post_fix, $col_fun_derator);
    }
  }
}

die "meta $metaName not exist!" if not exists $metaLib{$metaName};
analize_meta_processors($metaLib{$metaName}, "", "");

my $parser =  Spreadsheet::ParseExcel->new();
my $workbook = $parser->parse($inputFile);
if ( ! defined $workbook ) { die $parser->error(), ".\n"; }

sub read_sheet {
  my $sheet = shift;
  my $table = shift;

  my ( $row_min, $row_max ) = $sheet->row_range();
  my ( $col_min, $col_max ) = $sheet->col_range();

  if ( $row_max < 0 || $row_min > $row_max ) {
    die "sheet $inputSheet row range error!";
  }
  if ( $col_max < 0 || $col_min > $col_max ) {
    die "sheet $inputSheet col range error!";
  }

  my %tableHead = ();
  foreach my $colPos ($col_min .. $col_max) {
    my $colName;
    my $cell = $sheet->get_cell($row_min, $colPos);
    if ($cell) {
      $colName = $cell->value();
    }

    $tableHead{$colPos} = $colName;
  }

  foreach my $rowPos ( $row_min + $startLine .. $row_max ) {
    my %row;

    #跳过空行
    my $is_empty = 1;
    foreach my $colPos ( $col_min .. $col_max ) {
      my $cell = $sheet->get_cell($rowPos, $colPos);
      next if not defined $cell;
      next if not defined $cell->value() or $cell->value() =~ m/^\s*$/;
      $is_empty = 0;
      last;
    }

    last if $is_empty;

    my %input_row;
    foreach my $colPos ( $col_min .. $col_max ) {
      my $cell = $sheet->get_cell($rowPos, $colPos);
      next if not defined $cell;

      my $colName = $tableHead{$colPos};
      next if not defined $colName;

      $input_row{$colName} = $cell->value();
    }
    $input_row{tableName} = $inputSheet;

    foreach my $colName ( keys %input_row ) {
      next if not exists $input_col_processors{$colName};

      foreach my $processor ( @{$input_col_processors{$colName}} ) {
        $processor->(\%row, $input_row{$colName}, \%input_row);
      }
    }

    if (scalar(%row)) {
      if (exists ($row{is_valid}) ) {
        if ($row{is_valid} =~ m/^1$/) {
          delete ($row{is_valid});
          push @{ $table }, \%row;
        }
      } else {
        push @{ $table }, \%row;
      }
    }
  }
}

sub save_output {
  my $records = shift;
  my $output_path = shift;

  open(my $output, '>::encoding(utf8)', $output_path) or die "open output file $output_path fail $!";
  print $output Dump( $records );

}

sub process_sheet {
  my $sheet = shift;
  my $output_path = shift;
  my @table;

  read_sheet($sheet, \@table);

  my @no_emptty_role_table = ();
  foreach my $row (@table) {
    foreach my $value ( values %{$row} ) {
      if ($value !~ /^\s*$/) {  #goto next row if not empty
        push @no_emptty_role_table, $row;
        last;
      }
    }
  }

  save_output(\@no_emptty_role_table, $output_path);
}

my $main_file_generated = 0;

if ( defined $chanel ) {

  if ( $chanel eq "all" ) {
    my $match = "^$inputSheet\\|([\\w-]+)\$";

    foreach my $sheet ( $workbook->worksheets() ) {
      if ($sheet->get_name() =~ /$match/) {
        mkdir(dirname($outputFile) . "/" . $1);
        my $chanel_output = dirname($outputFile) . "/" . $1 . "/" . basename($outputFile);
        process_sheet($sheet, $chanel_output);
      }
      elsif ($sheet->get_name() eq $inputSheet) {
        $main_file_generated = 1;
        process_sheet($sheet, $outputFile);
      }
    }
  }
  else {
    my $sheet;
    if ( defined $chanel ) { $sheet =  $workbook->worksheet($inputSheet . "|" . $chanel); }
    if ( ! defined $sheet ) { $sheet = $workbook->worksheet($inputSheet); }
    if ( ! defined $sheet ) {
      die "sheet $inputSheet or $inputSheet|$chanel not exist in $inputFile!";
    }

    $main_file_generated = 1;
    process_sheet($sheet, $outputFile);
  }
}
else {
    my $sheet = $workbook->worksheet($inputSheet);
    if ( ! defined $sheet ) {
      die "sheet $inputSheet not exist in $inputFile!";
    }

    $main_file_generated = 1;
    process_sheet($sheet, $outputFile);
}

if (not $main_file_generated) {
  save_output([], $outputFile);
}

1;

# -*- encoding: utf-8 -*-
