CPDE_PERL_LIB:=$(CPDE_ROOT)/buildtools/perl
CPDE_PERL=LC_ALL=C PERL5LIB=$(call path-list-join,$(CPDE_PERL_LIB) $(PERL5LIB)) perl -w
