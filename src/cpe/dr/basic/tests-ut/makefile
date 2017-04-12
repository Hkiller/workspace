target-product:=gtest.cpe_dr
include ../../../../../buildtools/repository.mk
include ../../../../../buildtools/test-ut.mk


gen-target=$(if $(filter 64,$(cpe-dr-metalib)),input.bin.64,input.bin.32)

.PONEY: gen
gen: tsf4g-tools
	$(CPDE_OUTPUT_ROOT)/tools/bin/tdr -B -o $(gen-target) input.xml 2>&1 | iconv -f gb2312 -t utf-8
