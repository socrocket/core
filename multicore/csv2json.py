import csv

tmpl = """
/* %d */
{
  "conf" : {
    "prom" : {
      "elf" : "./rtems-ccsds123.mp%s.sparc"
    },
    "uart" : {
      "0" : {
        "index" : 1,
        "addr" : 1,
        "mask" : 4095,
        "irq" : 2,
        "type" : 0,
        "port" : %d
      }
    },
    "mmu_cache" : {
      "num" : %s
    },
    "ic" : {
      "en" : true,
      "repl" : 0,
      "sets" : %s,
      "linesize" : 4,
      "setsize" : %s,
      "setlock" : 1
    },
    "dc" : {
      "en" : true,
      "repl" : 2,
      "sets" : %s,
      "linesize" : 4,
      "setsize" : %s,
      "setlock" : 1,
      "snoop" : 1,
    }
  }
}
"""

reader = csv.reader(open('configurations-mp2.csv', 'rv'), delimiter=',', quotechar='"')
for row in reader:
  num = int(row[0])
  cpu = row[1]
  isets = row[2]
  isize = row[3]
  dsets = row[4]
  dsize = row[5]
  f = open('cfg.%03d.json' % num, "w")
  f.write(tmpl % (num, cpu, (7000+num), cpu, isets, isize, dsets, dsize))
  f.close()
