product-support-types+=android
product-def-all-items+=android.java-dir android.java-libs android.native-libs android.android-libs android.java-runtime-libs android.res-dir android.assets-dir android.manifest android.addition-datas android.generates android.abi

ANDROID_ARM_MODE?=arm
APKD?=1

ANDROID_ABIS:=armeabi armeabi-v7a

#$(call android-proj-copy-dir,src-dir,target-dir,postfix-list)
android-proj-copy-dir=copy-dir $1 $2 $3 android-proj-def-sep
android-proj-copy-file=copy-file $1 $2 android-proj-def-sep
android-proj-combine-etc-bin=combine-etc-bin $1 $2 android-proj-def-sep
android-proj-combine-etc-yml=combine-etc-yml $1 $2 android-proj-def-sep

.PHONY: android android.proj

# $(call product-def-rule-android-proj-copy,product-name,domain,src,target)
define product-def-rule-android-proj-copy

auto-build-dirs+=$(dir $4)

$1.$2.android.proj: $4

$4: $3
	$$(call with_message,copy $(patsubst $(CPDE_OUTPUT_ROOT)/$(if $2,$2/)%,%,$4))cp $$< $$@

endef

# $(call product-def-rule-android-proj-copy-dir,product-name,domain,args)
define product-def-rule-android-proj-copy-dir

$(call install-def-rule-one-dir-r,$1,$(strip $(word 1,$3)),$($1.$2.android.output)/$(strip $(word 2,$3)),$(wordlist 3,$(words $3),$3),$2,$1.$2.android.proj)

endef

# $(call product-def-rule-android-proj-copy-lib,product-name,domain,src,target)
define product-def-rule-android-proj-copy-lib

auto-build-dirs+=$(dir $4)

$1.$2.android.apk: $4

$4: $3 $1.$2.android.native
	$$(call with_message,copy $(patsubst $(CPDE_OUTPUT_ROOT)/$(if $2,$2/)%,%,$4))cp $$< $$@

endef


# $(call product-def-rule-android-proj-copy-file,product-name,domain,args)
define product-def-rule-android-proj-copy-file

$(call product-def-rule-android-proj-copy,$1,$2,$(CPDE_ROOT)/$(strip $(word 1,$3)),$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3)))

endef

# $(call product-def-rule-android-proj-combine-etc-yml,product-name,domain,args)
define product-def-rule-android-proj-combine-etc-yml

$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3))

$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3)): $$(CPE_CFG_TOOL) $(shell find $(CPDE_ROOT)/$(word 1,$3) -name "*.y[a]ml")
	$$(call with_message,combine $(word 1, $3) to $(word 2, $3)) $$(CPE_CFG_TOOL) combine --input $$(CPDE_ROOT)/$(word 1,$3) --output $$@ --format yml

endef

# $(call product-def-rule-android-proj-combine-etc-bin,product-name,domain,args)
define product-def-rule-android-proj-combine-etc-bin

$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3))

auto-build-dirs+=$(dir $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3)))

$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$(strip $(word 2,$3)): $$(CPE_CFG_TOOL) $(shell find $(CPDE_ROOT)/$(word 1,$3) -name "*.y[a]ml")
	$$(call with_message,combine $(word 1, $3) to $(word 2, $3))$$(CPE_CFG_TOOL) combine --input $$(CPDE_ROOT)/$(word 1,$3) --output $$@ --format bin

endef

# $(call product-def-rule-android-gen-dep,dep,dep-to,domain)
define product-def-rule-android-gen-dep
$1.$3.android.proj: $2.$3.android.proj

endef

# $(call product-def-rule-android-gen-depends-projs,product-name,domain)
define product-def-rule-android-gen-depends-projs
$(foreach d,$(call product-gen-depend-list,$($2.env),$1),\
   $(if $(filter lib,$($d.type)),\
       $(if $(filter $d,$(android.$2.defined-projects)),,\
            $(eval android.$2.defined-projects+=$d) \
            $(call product-def-rule-android-rules,$d,$2) \
        ) \
        $(call product-def-rule-android-gen-dep,$1,$d,$2) \
    )) \

endef

# $(call product-def-rule-android-gen-generate,product-name,domain,target,cmd)
define product-def-rule-android-gen-generate
$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$3

$(call $4,$1,$2,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/$3)

endef

# $(call product-def-rule-android-gen-java-rules,product-name,domain)
define product-def-rule-android-gen-java-rules

$(foreach m,$1 $(call product-gen-depend-list,$($2.env),$1),\
	$(foreach d,$(r.$m.android.generates),\
		$(call product-def-rule-android-gen-generate,$1,$2,$($m.android.generate.$d.target),$($m.android.generate.$d.cmd))))

$(foreach d,$(r.$1.android.java-dir) $(call product-gen-depend-value-list,$1,$($2.env),android.java-dir),\
    $(if $(wildcard $d),$(foreach j,$(shell find $d -name "*.java" -o -name "*.aidl"),\
        $(call product-def-rule-android-proj-copy,$1,$2,$j,$(patsubst $d/%,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/src/%,$j)))))

$(foreach d,$(r.$1.android.res-dir) $(call product-gen-depend-value-list,$1,$($2.env),android.res-dir),\
    $(if $(wildcard $d),$(foreach j,$(shell find $d -type f),\
        $(call product-def-rule-android-proj-copy,$1,$2,$j,$(patsubst $d/%,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/res/%,$j)))))

$(foreach d,$(r.$1.android.assets-dir) $(call product-gen-depend-value-list,$1,$($2.env),android.assets-dir),\
    $(if $(wildcard $d),$(foreach j,$(shell find $d -type f),\
        $(call product-def-rule-android-proj-copy,$1,$2,$j,$(patsubst $d/%,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/assets/%,$j)))))

$(foreach d,$(r.$1.android.java-libs) $(call product-gen-depend-value-list,$1,$($2.env),android.java-libs),\
	$(call product-def-rule-android-proj-copy,$1,$2,$d,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/libs/$(notdir $d)))

$(foreach d,$(r.$1.android.java-runtime-libs) $(call product-gen-depend-value-list,$1,$($2.env),android.java-runtime-libs),\
	$(call product-def-rule-android-proj-copy,$1,$2,$d,$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/runtime/$(notdir $d)))

$(foreach d,$(r.$1.android.native-libs) $(call product-gen-depend-value-list,$1,$($2.env),android.native-libs),\
	$(foreach p,$(if $($1.android.abi),$($1.android.abi),$(ANDROID_ABIS)), \
	    $(call product-def-rule-android-proj-copy-lib,$1,$2,\
              $(dir $d)$p/$(notdir $d), \
              $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/libs/$p/$(notdir $d)) \
         ))

endef

# $(call product-def-rule-android,product-name,domain)
define product-def-rule-android-app
$(eval $1.$2.android.stl?=gnustl_static)

.PHONY: $1.$2.android $1.android $2.android $1.$2.android.native

$2.android.apk android.apk $1.android.apk: $1.$2.android.apk

$1.$2.android.apk: $1.$2.android.native
	ant -f $$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/build.xml $(if $(filter 0,$(APKD)),release,debug)

$2.android.install android.install $1.android.install: $1.$2.android.install

$1.$2.android.install: $(if $(filter 1,$(only)),,$1.$2.android.apk)
	ant -f $$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/build.xml $(if $(filter 0,$(APKD)),installr,installd)

$1.$2.android.native: $(if $(filter 1,$(only)),,$1.$2.android.proj)
	$(NDK)/ndk-build $(if $(filter 0,$(APKD)),,NDK_DEBUG=1) APKD=$(APKD) $(if $V,V=$V) --directory=$$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output) $$(if $$(filter 1,$$V),V=1) -k

$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/local.properties \
                    $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/jni/Application.mk \
                    $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/AndroidManifest.xml \
                    $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/project.properties \
                    $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/build.xml

$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/AndroidManifest.xml: $(r.$1.$2.android.manifest) $(call product-gen-depend-value-list,$1,$2.env,android.manifest)
	$$(call with_message,$1.$2 <== generating $$(notdir $$@)) \
	$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-manifest.pl \
        --output $$@ \
        --input $$(r.$1.android.manifest) \
        --version $$(r.$1.version) \
        --package $$(r.$1.package) \
        $$(foreach a,$$(call product-gen-depend-value-list,$1,$2.env,android.addition-datas), --addition-data='$$a:$$($1.android.addition-data.$$a)') \
        $$(addprefix --merge , $(call product-gen-depend-value-list,$1,$2.env,android.manifest))

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/build.xml:
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo '<?xml version="1.0" encoding="UTF-8"?>' >> $$@
	$$(CPE_SILENCE_TAG)echo '<project name="$($1.android.name)-$($1.version)-$(client.chanel)" default="help">' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <property file="local.properties" />' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <property file="ant.properties" />' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <loadproperties srcFile="project.properties" />' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <fail' >> $$@
	$$(CPE_SILENCE_TAG)echo '            message="sdk.dir is missing. Make sure to generate local.properties using 'android update project' or to inject it through an env var"' >> $$@
	$$(CPE_SILENCE_TAG)echo '            unless="sdk.dir"' >> $$@
	$$(CPE_SILENCE_TAG)echo '    />' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <import file="custom_rules.xml" optional="true" />' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <path id="java.compiler.class.path">' >> $$@
	$$(CPE_SILENCE_TAG)$$(foreach ad,$(call product-gen-depend-value-list,$1,$2.env,android.java-runtime-libs),\
		echo '        <pathelement location="runtime/$$(notdir $$(ad))" />' >> $$@)
	$$(CPE_SILENCE_TAG)echo '    </path>' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <property name="java.compiler.classpath" value="$$$${toString:java.compiler.class.path}" />' >> $$@
	$$(CPE_SILENCE_TAG)echo '    <import file="$$$${sdk.dir}/tools/ant/build.xml" />' >> $$@
	$$(CPE_SILENCE_TAG)echo '</project>' >> $$@

#	$$(call with_message)

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/project.properties:
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'target=android-$$($1.android.version)' > $$@
	$$(CPE_SILENCE_TAG)$$(foreach ad,$(call product-gen-depend-value-list,$1,$2.env,android.android-libs),\
		echo 'android.library.reference.1=$$(call build-relative-path,$$(ad),$$@)' >> $$@)

$$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/local.properties:
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo "sdk.dir=$(ANDROID_HOME)" >> $$@
	$$(CPE_SILENCE_TAG)echo 'key.store=$($1.android.keystore)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'key.alias=$($1.android.keyalias)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'key.store.password=$($1.android.keystore_password)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'key.alias.password=$($1.android.keyalias_password)' >> $$@

$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/jni/Application.mk:
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_PATH := $$$$(call my-dir)' > $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_PLATFORM := android-9' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_STL := c++_static' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_ABI := $$(if $$($1.android.abi),$$($1.android.abi),$$(ANDROID_ABIS))' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_OPTIM := $$$$(if $$$$(filter 0,$$$$(APKD)),release,debug)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'APP_CFLAGS:=-Wno-extern-c-compat' >> $$@

$(eval product-def-rule-android-proj-tmp-name:=)
$(eval product-def-rule-android-proj-tmp-args:=)

$(foreach w,$($1.android.proj-src), \
    $(if $(filter android-proj-def-sep,$w) \
        , $(if $(product-def-rule-android-proj-tmp-name) \
              , $(call product-def-rule-android-proj-$(product-def-rule-android-proj-tmp-name),$1,$2,$(product-def-rule-android-proj-tmp-args)) \
                $(eval product-def-rule-android-proj-tmp-name:=) \
                $(eval product-def-rule-android-proj-tmp-args:=)) \
        , $(if $(product-def-rule-android-proj-tmp-name) \
              , $(eval product-def-rule-android-proj-tmp-args+=$w) \
              , $(eval product-def-rule-android-proj-tmp-name:=$w))))

endef

# $(call product-def-rule-android-rules,product-name,domain)
define product-def-rule-android-rules

$(if $(filter $1,$(android.$2.project-list)),$(warning $1 is already installed in android.$2),$(eval android.$2.project-list+=$1))

$(eval $1.$2.android.output:=$($2.output)/$1)
$(eval $1.$2.android.srcs:=$(subst $(CPDE_ROOT),../../../..,\
                           $(subst $(CPDE_OUTPUT_ROOT),../../..,\
                                   $(r.$1.c.sources) $(r.$1.$($2.env).c.sources) $(r.$1.$2.c.sources))))

auto-build-dirs+=$$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output) $$(CPDE_OUTPUT_ROOT)/$$($1.$2.android.output)/jni

.PHONY: $2.android.proj $1.android.proj $1.$2.android.proj

$2.android.proj android.proj $1.android.proj: $1.$2.android.proj

$1.$2.android.proj: $(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/jni/Android.mk $$(r.$1.$2.generated-sources)

$(CPDE_OUTPUT_ROOT)/$($1.$2.android.output)/jni/Android.mk: $$(r.$1.$2.symbols)
	$$(call with_message,$1.$2 <== generating $$(notdir $$@))echo '# anto generate by makefile' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_PATH := $$$$(call my-dir)' > $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo 'include $$$$(CLEAR_VARS)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_MODULE := $1' >> $$@
	$(if $(filter progn,$($1.type)),$$(CPE_SILENCE_TAG)echo 'LOCAL_EXPORT_CFLAGS += $$$$(if $$$$(filter 0,$$$$(APKD)),,-g)' >> $$@)
	$$(CPE_SILENCE_TAG)echo 'LOCAL_ARM_MODE := $(ANDROID_ARM_MODE)' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_CPP_FEATURES := rtti exceptions' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_CFLAGS += $$$$(if $$$$(filter 0,$$$$(APKD)),,-DDEBUG=1)' >> $$@
	$(if $(ASAN),$$(CPE_SILENCE_TAG)echo 'LOCAL_CFLAGS += -fsanitize=$(ASAN) -fno-omit-frame-pointer' >> $$@)
	$(if $(filter progn,$($1.type)),$(if $(ASAN),$$(CPE_SILENCE_TAG)echo 'LOCAL_LDFLAGS += -fsanitize=$(ASAN)' >> $$@))
	$$(CPE_SILENCE_TAG)echo 'LOCAL_CFLAGS += ' $$(call escape-quotes, \
                            $$(filter -D%,$$(call c-generate-depend-cpp-flags,$1,$2)) \
                            $$(r.$1.c.flags.cpp) $$(r.$1.$($2.env).c.flags.cpp) $$(r.$1.$2.c.flags.cpp) \
							$$(compiler.gcc.flags.warning) \
                            $$($($2.env).CFLAGS) \
                            $$(r.$1.$($2.env).c.flags.warning) \
                            $$(r.$1.c.flags.warning) \
                            $$(sort $$(call product-gen-depend-value-list,$1,$($2.env),product.c.flags.warning)) \
                            $$(r.$1.product.c.flags.warning) \
                            $$(r.$1.c.flags.lan.all) \
                            $$(r.$1.$2.c.flags.lan.all) \
                            $$(r.$1.c.flags.lan.c) \
                            $$(r.$1.$2.c.flags.lan.c) \
                            $$($($2.env).CPPFLAGS) \
                            $$($($2.env).TARGET_ARCH) \
                            )>> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_C_INCLUDES += ' $$(patsubst -I%,%,$$(filter -I%,$$(call c-generate-depend-cpp-flags,$1,$2))) >> $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$(if $(filter progn,$($1.type)),\
        $$(CPE_SILENCE_TAG)echo 'LOCAL_LDLIBS := -latomic $$$$(if $$$$(filter 0,$$$$(APKD)),,-g)' \
                                                $$(foreach f,$$(r.$1.c.export-symbols) $$(call product-gen-depend-value-list,$1,$2.env,c.export-symbols), $$(addprefix -u ,$$(shell cat $$(f)))) \
                                                $$(addprefix -L,\
                                                   $$(foreach ei,\
                                                      $$(call merge-list, $$(r.$1.c.ldpathes) \
                                                                        , $$(call product-gen-depend-value-list,$1,$($2.env),\
                                                                                $$(call c-generate-env-arg-name-list,$2,product.c.ldpathes)) \
                                                      ),\
                                                   $$(patsubst domain/%,$2/%,$$(subst /domain/,/$2/,$$(patsubst env/%,$($2.env)/%,$$(subst /env/,/$($2.env)/,$$(ei)))))) \
                                                ) \
                                                 $$(addprefix -l,\
                                                      $$(call product-gen-depend-value-list,$1,$($2.env),\
                                                          $$(call c-generate-env-arg-lib-name-list,$2,product.c.libraries))) \
                                                 $$(call revert-list,$$(call product-gen-depend-value-list,$1,$($2.env), \
                                                         $$(call c-generate-env-arg-name-list,$2,product.c.flags.ld))) \
                                                 $$($1.android.c.flags.ld) \
                           >> $$@)
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo 'LOCAL_SRC_FILES += $($1.$2.android.srcs)' >> $$@
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$(if $(filter progn,$($1.type)),\
	    $$(CPE_SILENCE_TAG)echo 'LOCAL_WHOLE_STATIC_LIBRARIES := ' \
                              $$(foreach d,$$(call product-gen-depend-list,$($2.env),$1),$$(if $$(filter lib,$$($$d.type)),$$d) $$(if $$($$d.android.mk),$$d)) \
                            >> $$@)
	$$(CPE_SILENCE_TAG)echo '' >> $$@
	$$(CPE_SILENCE_TAG)echo $(if $(filter progn,$($1.type)),'include $$$$(BUILD_SHARED_LIBRARY)','include $$$$(BUILD_STATIC_LIBRARY)') >> $$@
	$(if $(filter progn,$($1.type)),\
        $$(CPE_SILENCE_TAG)echo 'include ' \
                              $$(foreach d,$$(call product-gen-depend-list,$($2.env),$1),$$(if $$(filter lib,$$($$d.type)),$(CPDE_OUTPUT_ROOT)/$($2.output)/$$d/jni/Android.mk)) \
                              $$(foreach m,$$(call product-gen-depend-list,$($2.env),$1),$$($$m.android.mk)) \
                            >> $$@) \


$(if $(filter progn,$($1.type)),$(call product-def-rule-android-app,$1,$2))

endef

define product-def-rule-android_i

$(eval $2-android.output?=$($2.output)-android)
$(eval $2-android.ut?=0)
$(eval $2-android.env:=android)
$(eval $2-android.ignore-types:=android progn lib install)

$(call product-def-for-domain,$1,$2-android)

$(call product-def-rule-android-rules,$1,$2-android)

$(call post-commands-add,product-def-rule-android-gen-depends-projs,$1,$2-android)

$(call post-commands-add,product-def-rule-android-gen-java-rules,$1,$2-android)

endef

# $(call product-def-rule-android,product-name,domain)
define product-def-rule-android
$(if $(emscripten)$(flex),,$(call product-def-rule-android_i,$1,$2))
endef
