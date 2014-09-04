#!/usr/bin/perl -w

if ($#ARGV==0)
{
    $hmask  = $ARGV[0];
}
else
{
    print "\n usage: ./hmask2size hmask\n";
    exit;
}

if ($hmask =~ /[ ^0-9]/) {

    print "hmask not decimal - trying hex conversion\n";
    $hmask =~ s/"0x"//g;
    $hmask = hex($hmask);

}

$size = ((~$hmask & 0xfff)+1) << 20;

printf("Size of region: %x\n", $size);


