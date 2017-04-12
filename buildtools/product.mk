ifneq ($(CPDE_ROOT_INCLEDED),1)
$(error "include repository.mk first!"
endif

$(call assert-not-null,target-product)

all: $(target-product)

ut: $(foreach domain,$(sort $(using-domain-list)) \
          , $(if $(filter $($(target-product).ut),$($(domain).product-list)) \
                 , $(domain).$($(target-product).ut).run))

all: ut

clean: $(target-product).clean

.PHONY: clean-all
clean-all:
	$(call with_message,"clean all...")$(RM) -r $(CPDE_OUTPUT_ROOT)