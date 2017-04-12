# $(call assert,condition,message)
define assert
  $(if $1,,$(error Assertion failed: $2))
endef

# $(call assert-file-exists,wildcad-pattern)
define assert-file-exists
  $(call assert,$(wildcard $1),$1 does not exit)
endef

# $(call assert-not-null,make-variable)
define assert-not-null
  $(call assert,$($1),The variable "$1" is null)
endef

# $(call assert-set-one,make-variable-list)
define assert-set-one
  $(if $(filter 1,$(words $(foreach m,$1,$($m)))),,$(warning $1 only can set 1 value))
endef

# $(call concat,a b c d)
concat=$(if $(firstword $1),$(firstword $1)$(call concat,$(wordlist 2,$(words $1),$1)))

# $(call join-path,part1,part2)
join-path=$(if $(1),$(strip $(1))/$(strip $(2)),$(2))

ifneq ($(V),1)
  define with_message
    @$(if $1,echo ">>> $1" &&, )
  endef

  CPE_SILENCE_TAG:=@
endif

ifeq ($(MKD),1)
  debug-warning=$(warning $1)
  debug-variables=$(warning $1)
endif

# $(call stack-push,stack-variable-name,word)
define stack-push
$1 := $2 $($1)
endef

# $(call stack-pop,stack-variable-name)
define stack-pop
$1 := $(wordlist 2,$(words $($1)),$($1))
endef

# $(call build-regular-path,output-variable,input-variable)
define build-regular-path
$(foreach part,$(subst /,$(space),$2),                          \
    $(if $(filter .,$(part)),,                                  \
    $(if $(filter ..,$(part)),$(eval $(call stack-pop,$1.inbuild))      \
                             ,$(eval $(call stack-push,$1.inbuild,$(part)))))   \
)
$(foreach result-part,$($1.inbuild),$(eval $1:=/$(result-part)$($1)))
endef

# $(call list-to-path,path)
list-to-path=$(if $(word 1,$1),$(if $(word 2,$1),$(word 1,$1)/$(call list-to-path,$(wordlist 2,$(words $1),$1)),$1))

# $(call build-relative-path,path,base)
build-relative-path-last=$(if $(word 1,$1) \
                           , $(if $(word 1, $2) \
                                 , $(if $(filter $(word 1,$1),$(word 1,$2)) \
                                        , $(call build-relative-path-last,$(wordlist 2,$(words $1),$1),$(wordlist 2,$(words $2),$2)) \
                                        , $1 \
                                    ) \
                                 , $1 \
                              ) \
                        )

build-relative-path-node-to-up=$(if $(word 2,$1),.. $(call build-relative-path-node-to-up,$(wordlist 2,$(words $1),$1)))

build-relative-path-prefix=$(if $(word 1,$1) \
                           , $(if $(word 1, $2) \
                                 , $(if $(filter $(word 1,$1),$(word 1,$2)) \
                                        , $(call build-relative-path-prefix,$(wordlist 2,$(words $1),$1),$(wordlist 2,$(words $2),$2)) \
                                        , $(call build-relative-path-node-to-up,$2) \
                                    ) \
                              ) \
                           )

build-relative-path=$(call list-to-path,$(call build-relative-path-prefix,$(subst /, ,$1),$(subst /, ,$2))$(call build-relative-path-last,$(subst /, ,$1),$(subst /, ,$2)))

# $(call path-list-join,path-list)
define path-list-join
$(if $1,$(word 1,$1):$(call path-list-join,$(wordlist 2,$(words $1), $1)),)
endef

# $(call append-if-need,list,var)
append-if-need=$(if $(filter $(strip $2),$1),$1,$1 $(strip $2))

# $(call apend-force-to-end,list,var)
append-force-to-end=$(filter-out $(strip $2),$1) $(strip $2)

# $(call merge-list,list,list2)
merge-list=$(if $2,$(call append-if-need,$(call merge-list,$1,$(wordlist 2,$(words $2),$2)),$(word 1,$2)),$1)

# $(call merge-list,list,list2)
merge-force-to-end=$(filter-out $2, $1) $2

# $(call revert-list,$1)
revert-list=$(if $1,$(call revert-list,$(wordlist 2,$(words $1),$1)) $(word 1,$1))

# $(call regular-list,$1)
regular-list=$(if $1,$(word 1,$1) $(call regular-list,$(wordlist 2,$(words $1),$1)))

# $(call select-var,var-list)
define select-var
$(if $1\
     , $(warning $(word 1,$1))\
	   $(if $($(word 1, $1)) \
          , $($(word 1, $1))\
          , $(call select-var,$(wordlist 2,$(words $1), $1))) \
     ,)
endef

compiler-category=$(strip \
                      $(if $(filter 1,$(flex)),gcc-flex, \
                      $(if $(filter %clang,$1),clang,\
                      $(if $(filter %clang++,$1),clang,\
                      $(if $(filter %gcc %g++ %gcc44 %g++44,$1),gcc,\
                      $(if $(filter %emcc %em++,$1),emcc,\
                      $(warning unknown compiler $1)))))))

# $(call filter-out-substring,substring,text)
filter-out-substring=$(foreach v,$2,$(if $(findstring $1,$v),,$v))

# $(call rwildcard,path,patterns)
rwildcard=$(call filter-out-substring,\#,$(shell find $1 -name "" $(foreach d,$2,-or -name "$d")))

# $(call check-content,file,content)
check-content=$(if $(wildcard $1),$(if $(filter $2,$(shell cat $1)),0,1$(shell echo '$2' > $1)),1$(shell echo '$2' > $1))

__quotes_origin:="
__quotes_escape:=\"
escape-quotes=$(subst $(__quotes_origin),$(__quotes_escape),$1)
