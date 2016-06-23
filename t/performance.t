#!/usr/bin/perl
use Test::More;
use IO::Socket;
use Time::HiRes qw( usleep gettimeofday tv_interval );

#local $| = 1;

ok `pgrep adkvs`, 'process is running';

my $sock = IO::Socket::INET->new(PeerAddr=>'127.0.0.1',PeerPort=>1985,Proto=>'tcp') or die $!;
my $v = "a"x10;

my $t0sum = 0;
my $ttl_cnt = 500000;
for (1..$ttl_cnt) {
    my $t0 = [gettimeofday];
    $sock->print("set preins$_ $_\n");
    $sock->getline;
#    print "\b"x1000;
    my $elapsed = tv_interval($t0);
    
    if ($_ % 1000 == 0) {
        print $_.",".int($t0sum)."ms\n";
        $t0sum = 0;
    } else {
       $t0sum += $elapsed*1000;
    }
}

print "\ncheck datas...";

for (1..$ttl_cnt) {
    $sock->print("get preins$_\n");
    my $v = $sock->getline;
    chomp $v;
    die "err value at index:$_,  v=$v, _=$_;" if ($v != $_);
}
print " ok!\n";

for my $l (1..10) {
    my $t0 = [gettimeofday];
    for (1..10000) {
        $sock->print("set hoge$_ $v\n");
        $sock->getline;
    }
    my $elapsed = tv_interval($t0);
    note int($elapsed * 1000), "ms at $l times 10000";
    for (1..10000) {
        $sock->print("get hoge$_\n");
        my $_v = $sock->getline;
        chomp $_v if ($_v);
        if ($_v ne $v) {
            diag "get:$_v   need:$v\n";
        }
    }
}

my $t0 = [gettimeofday];
for (1..10000) {
    $sock->print("get hoge$_\n");
    my $_v = $sock->getline;
    chomp $_v if ($_v);
    if ($_v ne "(NULL)") {
        diag "get:$_v   need:(NULL)\n";
    }
}
my $elapsed = tv_interval($t0);
note int($elapsed * 1000), "ms at undefiend keys get 10000";

$sock->close;

done_testing;
