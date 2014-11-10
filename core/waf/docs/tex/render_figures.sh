#!/bin/bash

figures="ahb_apb_timing ahb_lt_timing ahb_at_timing_burst ahb_at_timing_overlap ahb_at_timing_single"

for figure in $figures
do
  echo "running pdflatex for $figure"
  pdflatex "$figure".tex >/dev/null
  echo "converting pdf to svg"
  pdf2svg "$figure".pdf "$figure".svg
done
