#!/bin/sh

echo '' > tsim_results.txt

for bench in `ls --ignore=*.c io_less`
do
    echo '*****************************************************' >> tsim_results.txt
    echo $bench >> tsim_results.txt
    #tsim-hw-at697 -hwbp -nouart io_less/$bench >> tsim_results.txt
    #tsim-leon -fast_uart -hwbp -nfp -sdram 0 -nouart -ram 8192 io_less/$bench >> tsim_results.txt
    ./grsim.exe -simconf ../../examples/simconf.so -fast_uart -sram 10240 -c /home/luke/.tsimrc -f io_less/$bench >> tsim_results.txt
done
