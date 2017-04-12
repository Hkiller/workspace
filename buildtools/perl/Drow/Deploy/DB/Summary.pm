package Drow::Deploy::DB::Summary;
use YAML;
use strict;
use Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(generate_summary_host generate_summary_word);

sub generate_summary_host {
  (my $self, my $output_dir) = @_;

  my $rv = 1;

  my $hosts = $self->{hosts};
  foreach my $host ( values $hosts ) {
    my $host_info = { name => $host->{name}
                      , ip => $host->{ip}
                      , worlds => $host->{worlds}
                    };

    #ports
    my @host_ports_info = ();
    foreach my $port ( keys $host->{ports} ) {
      push @host_ports_info, { port => $port, user => $host->{ports}->{$port}->{user} };
    }
    @host_ports_info = sort { $a->{user} cmp $b->{user} } @host_ports_info;
    $host_info->{ports} = \@host_ports_info;

    #输出
    my $output_file = "$output_dir/host-" . $host->{name} . ".yml";
    if (open(my $output, '>::encoding(utf8)', $output_file)) {
      print $output Dump( $host_info );
      close $output;
    }
    else {
      print "generate summary host: write to $output_file fail $!\n";
      $rv = 0;
    }
  }

  return $rv;
}

sub generate_summary_word {
  (my $self, my $output_dir) = @_;

  my $rv = 1;

  my $worlds = $self->{worlds};
  foreach my $world ( values $worlds ) {
    my $world_info = { name => $world->{name}
                    };

    my $world_regions_info = $world_info->{regions};
    foreach my $region_id ( keys $world->{regions} ) {
      my $region = $world->{regions}->{$region_id};
      push @{$world_regions_info}, { id => $region_id };
    }

    my @world_sets_info = ();
    foreach my $set_id ( keys $world->{sets} ) {
      my $set = $world->{sets}->{$set_id};
      my $set_info = { id => $set_id, type => $set->{type} };
      $set_info->{region} = $set->{region} if ($set->{region});
      push @world_sets_info, $set_info;
    }
    @world_sets_info = sort { $a->{id} <=> $b->{id} } @world_sets_info;
    $world_info->{sets} = \@world_sets_info;

    my $output_file = "$output_dir/world-" . $world->{name} . ".yml";

    if (open(my $output, '>::encoding(utf8)', $output_file)) {
      print $output Dump( $world_info );
      close $output;
    }
    else {
      print "generate summary world: write to $output_file fail $!\n";
      $rv = 0;
    }
  }

  return $rv;
}


