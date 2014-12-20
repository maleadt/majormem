#!/usr/bin/perl

use strict;
use warnings;

use List::Util qw/max/;

use Data::Dumper;

open(my $dict, '<', 'dutch.dat');

my @letters = qw/s t n m r l d k f p/;

my $string = shift || die("Geef nummerreeks op");

# Find words matching strings of numbers
sub find_words {
    my $combination = shift;

    my $joker = '[aeiou]*';
    my $regex = '^' . $joker;
    for my $c (split(//, $combination)) {
        $regex .= $letters[$c] . $joker;
    }
    $regex .= '$';
    $regex = qr/$regex/;

    my @words;
    seek($dict, 0, 0);
    while (<$dict>) {
        chomp;
        next if length($_) < 2;
        push @words, $_ if $_ =~ $regex;
    }

    return @words;
}

# Split long number series into all possible combinations
sub find_combinations {
    my $string = shift;
    my $inner = shift || 0;

    # TODO: bail out of none of the child calls managed a combination

    my %combinations;

    for my $len (1 .. length($string)) {
        print "  step $len of ", length($string), "\n" if !$inner;
        my $substring = substr($string, 0, $len);
        my $subwords = join("|", find_words($substring));
        if ($subwords) {
            my $remainder = substr($string, $len);
            if (length($remainder)) {
                my %remcombinations = find_combinations(substr($string, $len), 1);
                $combinations{$subwords} = \%remcombinations;
            } else {
                $combinations{$subwords} = undef;
            }
        };
    }

    return %combinations;
}

# Reduce nested-hash constructions to pure lists with all items expanded
sub squash {
    my @initial = @{$_[0]};
    my %combinations = %{$_[1]};

    my @squashed;
    for my $string (keys(%combinations)) {
        my @subsequent = (@initial, $string);

        if ($combinations{$string}) {
            push @squashed, squash(\@subsequent, $combinations{$string});
        } else {
            push @squashed, [@subsequent];
        }
    }
    return @squashed;
}

# Print squashed lists in a nice tabular format
sub print_squashed {
    my @combinations = @_;

    my @lengths;
    my @split_combinations;
    foreach my $combination (@combinations) {
        my @words = sort { length($a) <=> length($b) } split("\\|", $combination);
        push @split_combinations, \@words;
        push @lengths, max map { length($_) } @words;
    }

    my $max_wordcount = max map { scalar(@{$_}) } @split_combinations;

    for (my $row = 0; $row < $max_wordcount; $row++) {
        for (my $column = 0; $column < @combinations; $column++) {
            my $word = "";
            if ($row < scalar(@{$split_combinations[$column]})) {
                $word = $split_combinations[$column][$row];
            } else {
                $word = " "x$lengths[$column];
            }

            my $collength = $lengths[$column]+3;
            printf "%${collength}s", $word;
        }
        print "\n";
    }

    print "\n\n";
}

sub combination_cost {

}

print "Finding combinations...\n";
my %combinations = find_combinations($string);

print "Visualizing...\n";
my @squashed = sort { scalar(@{$b}) <=> scalar(@{$a}) } squash([], \%combinations);
for (@squashed) {
    print_squashed(@{$_});
}

close($dict);
