package Drow::Deploy::DB;
use strict;
use Spreadsheet::ParseExcel;
use Exporter;
use Drow::Utils::Excel;
use Drow::Deploy::DB::Parse;
use Drow::Deploy::DB::Summary;

our @ISA = qw(Exporter);
our @EXPORT = qw(have_error print_error dump_summary world);
our @EXPORT_OK = qw(append_error);

sub new {
  my $class = shift;
  my $self = { worlds => {}
               , dbs => {}
               , regions => []
               , sets => []
               , hosts => {}
               , errors => []
               , args => {}
               , chanels => []
               , versions => {}
             };
  bless $self, ref $class || $class;
  return $self;
}

sub parse {
  (my $class, my $file_name) = @_;

  my $self = $class->new();
  return $self if ! $self;

  my $parser =  Spreadsheet::ParseExcel->new();
  my $workbook = $parser->parse($file_name);
  if ( ! defined $workbook ) {
    print $parser->error(), ".\n";
    return;
  }

  $self->{file} = $file_name;

  my $args = read_table($workbook, "Setup", 2) or return;
  my $chanels = read_table($workbook, "Chanel", 2) or return;
  my $versions = read_table($workbook, "Version", 2) or return;
  my $worlds = read_table($workbook, "World", 2) or return;
  my $dbs = read_table($workbook, "Db", 2) or return;
  my $hosts = read_table($workbook, "Host", 2) or return;
  my $regions = read_table($workbook, "Region", 2) or return;
  my $sets = read_table($workbook, "Set", 2);

  if ( ! $self->parse_args($args)
       || ! $self->parse_chanels($chanels)
       || ! $self->parse_versions($versions)
       || ! $self->parse_hosts($hosts)
       || ! $self->parse_dbs($dbs)
       || ! $self->parse_worlds($worlds)
       || ! $self->parse_regions($regions)
       || ! $self->parse_sets($sets)
        )
  {
    return;
  }

  return $self;
}

sub world {
  (my $self, my $world_name) = @_;

  my $worlds = $self->{worlds};

  $self->append_error("world $world_name not exist!") and return if not exists $worlds->{$world_name};

  return $worlds->{$world_name};
}

sub append_error {
  (my $self, my $msg, my $location) = @_;

  my $error = { msg => $msg };

  $error->{location} = $location if (defined $location);

  push @{$self->{errors}}, $error;
}

sub have_error {
  my $self = shift;

  return @{ $self->{errors} } > 0;
}

sub print_error {
  my $self = shift;

  foreach my $err (@{ $self->{errors} }) {
    print $err->{msg} . "     (" . $err->{location}->{sheet} . ":" . $err->{location}->{row} . ")\n";
  }
}

sub generate_summary {
  (my $self, my $output_dir) = @_;
  my $rv = 1;

  $rv = 0 if ! $self->generate_summary_host($output_dir);
  $rv = 0 if ! $self->generate_summary_word($output_dir);

  return $rv;
}

1;
