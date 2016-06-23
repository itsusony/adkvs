#!/usr/bin/perl
use Test::More;
use IO::Socket;

ok `pgrep adkvs`, 'process is running';

my $sock = IO::Socket::INET->new(PeerAddr=>'127.0.0.1',PeerPort=>1985,Proto=>'tcp') or die $!;

subtest "ping" => sub {
    $sock->print("ping\n");
    is $sock->getline, "pong\n", 'ping pong test';
};

subtest "set & get" => sub {
    $sock->print("set hoge fuga\n");
    is $sock->getline, "(OK)\n", 'set test';
    
    $sock->print("get hoge\n");
    is $sock->getline, "fuga\n", 'get test';

    $sock->print("set hoge \n");
    is $sock->getline, "(OK)\n", 'set empty test';
    
    $sock->print("get hoge\n");
    is $sock->getline, "\n", 'get empty test';
};

subtest "get twice test, need return nothing" => sub {
    $sock->print("set hoge fuga\n");
    is $sock->getline, "(OK)\n", 'set test';
    
    $sock->print("get hoge\n");
    is $sock->getline, "fuga\n", 'get test';
    $sock->print("get hoge\n");
    is $sock->getline, "(NULL)\n", 'get twice test';
};

subtest "set & get key big than 32" => sub {
    my $cmd = "set key".('a' x 32)." 1\n";
    note $cmd;
    $sock->print($cmd);
    is $sock->getline, "(ERR)\n", 'set big key test';

    $cmd = "get key".('a' x 32)."\n";
    note $cmd;
    $sock->print($cmd);
    is $sock->getline, "(ERR)\n", 'get big key test';
};

subtest "set big value than 2048" => sub {
    my $cmd = "set key1 ".("1" x 2048)."\n";
    $sock->print($cmd);
    is $sock->getline, "(OK)\n", 'set big key just length 2048';

    $cmd = "set key1 ".("1" x 2049)."\n";
    $sock->print($cmd);
    is $sock->getline, "(ERR)\n", 'set big key more than length 2048';
};

$sock->close;

done_testing;
