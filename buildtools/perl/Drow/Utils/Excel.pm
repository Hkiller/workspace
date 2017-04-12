package Drow::Utils::Excel;
use strict;
use Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(read_table);

sub read_table {
  my ($workbook, $sheet_name, $start_line) = @_;

  my $sheet = $workbook->worksheet($sheet_name);
  if ( ! defined $sheet ) {
    print "sheet $sheet_name not exist!\n";
    return 0;
  }

  my ( $row_min, $row_max ) = $sheet->row_range();
  my ( $col_min, $col_max ) = $sheet->col_range();
  if ( $row_max < 0 || $row_min > $row_max ) { print "sheet $sheet_name row range error!\n" and return; }
  if ( $col_max < 0 || $col_min > $col_max ) { print "sheet $sheet_name col range error!\n" and return; }

  my $table = [];

  my %heads = ();
  foreach my $col_pos ($col_min .. $col_max) {
    my $cell = $sheet->get_cell($row_min, $col_pos);
    if ($cell) {
      $heads{$col_pos} = $cell->value();
    }
    else {
      last;
    }
  }

  foreach my $row_pos ( $row_min + $start_line .. $row_max ) {
    my $record = { };
    my $is_empty = 1;

    foreach my $col_pos ($col_min .. $col_max) {
      my $cell = $sheet->get_cell($row_pos, $col_pos);
      if ($cell && exists $heads{$col_pos} && $cell->value() !~ '/^\s*$/') {
        $record->{$heads{$col_pos}} = $cell->value();
        $is_empty = 0;
      }
    }

    next if $is_empty;

    $record->{_location} = { sheet => $sheet_name, row => $row_pos };

    push @{ $table }, $record;
  }

  return $table;
}

1;
