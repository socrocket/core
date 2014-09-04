#!/usr/bin/perl -w

if ($#ARGV==1)
{
    $haddr  = $ARGV[0];
    $hmask = $ARGV[1];
}
else
{
    print "\n usage: ./haddr2addr.pl haddr mask\n";
    exit;
}

if ($haddr =~ /[ ^0-9]/) {

    print "haddr not decimal - trying hex conversion\n";
    $haddr =~ s/"0x"//g;
    $haddr = hex($haddr);

}

if ($hmask =~ /[ ^0-9]/) {

    print "hmask not decimal - trying hex conversion\n";
    $hmask =~ s/"0x"//g;
    $hmask = hex($hmask);

}

$haddr = $haddr & 0xfff;
$hmask = $hmask & 0xfff;

$address = ($haddr & $hmask) << 20;

printf("AHB address: %x\n", $address);


