#!/usr/bin/env perl

use strict;
use warnings;
use v5.10;

use Data::Dumper;
use File::Slurper qw(read_text);
use TOML qw(from_toml);

if (@ARGV != 1) {
    die "specify a file to dump the contents of";
}

say Dumper(from_toml(read_text($ARGV[0])));
