
define post-commands-add

post-commands-list+=$1 $2 $3 $4 $5 $6 $7 $8 $9 $(10) post-commands-def-sep

endef 

define post-commands-execute

$(eval post-commands-execute-tmp-name:=)
$(eval post-commands-execute-tmp-args:=)

$(foreach w,$(post-commands-list), \
    $(if $(filter post-commands-def-sep,$w) \
        , $(if $(post-commands-execute-tmp-name) \
              , $(call $(post-commands-execute-tmp-name)\
                       ,$(strip $(word 1,$(post-commands-execute-tmp-args))),$(strip $(word 2,$(post-commands-execute-tmp-args))),$(strip $(word 3,$(post-commands-execute-tmp-args))),$(strip $(word 4,$(post-commands-execute-tmp-args))),$(strip $(word 5,$(post-commands-execute-tmp-args))),$(strip $(word 6,$(post-commands-execute-tmp-args))),$(strip $(word 7,$(post-commands-execute-tmp-args))),$(strip $(word 8,$(post-commands-execute-tmp-args))),$(strip $(word 9,$(post-commands-execute-tmp-args)))) \
                $(eval post-commands-execute-tmp-name:=) \
                $(eval post-commands-execute-tmp-args:=)) \
        , $(if $(post-commands-execute-tmp-name) \
              , $(eval post-commands-execute-tmp-args+=$w) \
              , $(eval post-commands-execute-tmp-name:=$w))))

endef
