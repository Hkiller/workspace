
.PHONY: gtags clean-gtags
gtags:
	$(call with_message,generating gtags...)cd $(CPDE_ROOT) && gtags .

clean-gtags:
	$(call with_message,cleaning gtags...)cd $(CPDE_ROOT) && $(RM) GPATH GRTAGS GTAGS GSYMS
