#!/usr/bin/perl
use IO::Socket;
use Benchmark qw(:all);

my $sock = IO::Socket::INET->new(
    PeerAddr=>'127.0.0.1',
    PeerPort=>1985,
    Proto=>'tcp'
) or die $!;

my $i = 0;

timethese(1000000,{
    "normal" => sub {
        $sock->print("set key$i val$i\n");
        $sock->getline;
        $i++;
    }
});

$sock->close;
