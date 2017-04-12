# {{{ 注册到product-def.mk

product-support-types+=ui-model

# }}}

# {{{ 定义cocosModule导入

#$(call install-ui-model-cocos-module,product,module,to-path,pics)
define install-ui-model-cocos-module

$(strip $1).ui-model.cocos-module.modules += $2
$(strip $1).ui-model.cocos-module.$(strip $2).input=$4
$(strip $1).ui-model.cocos-module.$(strip $2).output=$3
$(strip $1).ui-model.cocos-module.$(strip $2).input-base:=$(if $5,$5/,$(firstword $(sort $(dir $4))))

endef

# }}}
# {{{ 定义cocos序列帧特效

#$(call install-ui-model-cocos-effects-one,product,module,src-path,to-path,frame-duration,frame-position,frame-order)
define install-ui-model-cocos-effect-one
$1.ui-model.cocos-effect.modules+=$2
$1.ui-model.cocos-effect.$2.input=$3/$2
$1.ui-model.cocos-effect.$2.output=$(if $4,$4/$2,$2)
$1.ui-model.cocos-effect.$2.frame-duration=$5
$1.ui-model.cocos-effect.$2.frame-position=$6
$1.ui-model.cocos-effect.$2.frame-order=$7

endef

#$(call install-ui-model-cocos-effects,product,src-path,to-path,frame-duration,frame-position,frame-order)
define install-ui-model-cocos-effects
$(foreach m,$(basename $(notdir $(wildcard $($1.ui-model.root)/$2/*.plist))),$(call install-ui-model-cocos-effect-one,$1,$m,$2,$3,$4,$5,$6))

endef

# }}}
# {{{ 定义cocos2特效

#$(call install-ui-model-cocos-particles,product,module,src-path,to-path)
define install-ui-model-cocos-particle-one
$1.ui-model.cocos-particle.modules+=$2
$1.ui-model.cocos-particle.$2.input=$3/$2
$1.ui-model.cocos-particle.$2.output=$(if $4,$4/$2,$2)

endef

define install-ui-model-cocos-particles
$(foreach m,$(basename $(notdir $(wildcard $($1.ui-model.root)/$2/*.plist))),$(call install-ui-model-cocos-particle-one,$1,$m,$2,$3,$4))

endef

# }}}
# {{{ 定义Module导入

#$(call install-ui-model-module,product,module,to-path,pics)
define install-ui-model-module

$(strip $1).ui-model.module.modules += $2
$(strip $1).ui-model.module.$(strip $2).input=$4
$(strip $1).ui-model.module.$(strip $2).output=$3
$(strip $1).ui-model.module.$(strip $2).input-base:=$(if $5,$5/,$(firstword $(sort $(dir $4))))

endef

# }}}
# {{{ 定义file序列帧特效

#$(call install-ui-model-file-effects-one,product,module,to-path,input-path,pic-fils,frame-duration,frame-position,frame-order)
define install-ui-model-file-effect-one
$1.ui-model.file-effect.modules+=$2
$1.ui-model.file-effect.$2.pic-files=$5
$1.ui-model.file-effect.$2.input=$4
$1.ui-model.file-effect.$2.output=$3
$1.ui-model.file-effect.$2.frame-duration=$6
$1.ui-model.file-effect.$2.frame-position=$7
$1.ui-model.file-effect.$2.frame-order=$8

endef
#                                     $1      $2      $3       $4  $5             $6             $7
#$(call install-ui-model-file-effects,product,to-path,src-path,sep,frame-duration,frame-position,frame-order)
define install-ui-model-file-effects

$(eval install-ui-model-file-effects-file-list:=$(basename $(notdir $(wildcard $3/*.png))))

$(foreach m,\
    $(sort $(foreach f,$(install-ui-model-file-effects-file-list),$(firstword $(subst $4, ,$f)))), \
    $(call install-ui-model-file-effect-one,$1,$m,$(if $2,$2/$m,$m),$3,$(filter $m$4%,$(install-ui-model-file-effects-file-list)),$5,$6,$7)) \


endef

# }}}
# {{{ 定义dir序列帧特效

#                                    $1      $2     $3      $4       $5             $6             $7 
#$(call install-ui-model-dir-effects,product,module,to-path,src-path,frame-duration,frame-position,frame-order)
define install-ui-model-dir-effects

$1.ui-model.dir-effect.modules+=$2
$1.ui-model.dir-effect.$2.input=$4
$1.ui-model.dir-effect.$2.output=$3
$1.ui-model.dir-effect.$2.frame-duration=$5
$1.ui-model.dir-effect.$2.frame-position=$6
$1.ui-model.dir-effect.$2.frame-order=$7

endef

# }}}
# {{{ 定义spine动画导入

#                                   $1      $2     $3           $4         $5            $6
#$(call install-ui-model-spine-anim,product,module,output-spine,output-pic,output-module,src-file)
define install-ui-model-spine-anim

$(strip $1).ui-model.spine-anim.modules+=$(strip $2)
$(strip $1).ui-model.spine-anim.$(strip $2).input:=$(strip $6)
$(strip $1).ui-model.spine-anim.$(strip $2).output-spine:=$(strip $3)
$(strip $1).ui-model.spine-anim.$(strip $2).output-pic:=$(strip $4)
$(strip $1).ui-model.spine-anim.$(strip $2).output-module:=$(strip $5)

endef

# }}}
# {{{ 定义CrazyStorm弹幕导入

#                                           $1      $2     $3     $4       $5
#$(call install-ui-model-crazystorm-barrage,product,module,output,src-file,bullets)
define install-ui-model-crazystorm-barrage

$(strip $1).ui-model.crazystorm-barrage.modules+=$(strip $2)
$(strip $1).ui-model.crazystorm-barrage.$(strip $2).input:=$(strip $4)
$(strip $1).ui-model.crazystorm-barrage.$(strip $2).output:=$(strip $3)
$(strip $1).ui-model.crazystorm-barrage.$(strip $2).bullets:=$(strip $5)

endef

# }}}
# {{{ 定义Chipmunk从Sprite导入

#                                             $1      $2     $3     $4
#$(call install-ui-model-chipmunk-from-sprite,product,module,output,input)
define install-ui-model-chipmunk-from-sprite

$(strip $1).ui-model.chipmunk-from-sprite.modules+=$(strip $2)
$(strip $1).ui-model.chipmunk-from-sprite.$(strip $2).input:=$(strip $4)
$(strip $1).ui-model.chipmunk-from-sprite.$(strip $2).output:=$(strip $3)

endef

# }}}
# {{{ 定义Chipmunk从RUBE导入

#                                             $1      $2     $3     $4  $5
#$(call install-ui-model-chipmunk-from-rube,product,module,output,input,ptm)
define install-ui-model-chipmunk-from-rube

$(strip $1).ui-model.chipmunk-from-rube.modules+=$(strip $2)
$(strip $1).ui-model.chipmunk-from-rube.$(strip $2).input:=$(strip $4)
$(strip $1).ui-model.chipmunk-from-rube.$(strip $2).output:=$(strip $3)
$(strip $1).ui-model.chipmunk-from-rube.$(strip $2).ptm:=$(strip $5)

endef

# }}}
# {{{ 定义Chipmunk从Chipmunk导入

#                                               $1      $2     $3     $4    $5       $6
#$(call install-ui-model-chipmunk-from-chipmunk,product,module,output,input,adj-base,ptm)
define install-ui-model-chipmunk-from-chipmunk

$(strip $1).ui-model.chipmunk-from-chipmunk.modules+=$(strip $2)
$(strip $1).ui-model.chipmunk-from-chipmunk.$(strip $2).input:=$(strip $4)
$(strip $1).ui-model.chipmunk-from-chipmunk.$(strip $2).output:=$(strip $3)
$(strip $1).ui-model.chipmunk-from-chipmunk.$(strip $2).adj-base:=$(strip $5)
$(strip $1).ui-model.chipmunk-from-chipmunk.$(strip $2).ptm:=$(strip $6)

endef

# }}}
# {{{ 定义Tiledmap到图片的导出

#                                        $1      $2     $3     $4    $5
#$(call install-ui-model-tiledmap-to-pic,product,module,output,input,layers)
define install-ui-model-tiledmap-to-pic

$(strip $1).ui-model.tiledmap-to-pic.modules+=$(strip $2)
$(strip $1).ui-model.tiledmap-to-pic.$(strip $2).input:=$(strip $4)
$(strip $1).ui-model.tiledmap-to-pic.$(strip $2).output:=$(strip $3)
$(strip $1).ui-model.tiledmap-to-pic.$(strip $2).layers:=$(strip $5)

endef

# }}}
# {{{ 定义TiledMap场景导入导出

#                                             $1      $2     $3
#$(call install-ui-model-scene-proj,product,module,output,project-path)
define install-ui-model-scene-proj

$(strip $1).ui-model.scene-proj.modules+=$(strip $2)
$(strip $1).ui-model.scene-proj.$(strip $2).proj-path:=$(strip $3)

endef

# }}}
# {{{ 实现：模型转换脚本

#$(call def-ui-model-tool-op,product,op,op-file)
define def-ui-model-tool-op

.PHONY: $1-model-op-$2
$1-model-op-$2: $$(UI_MODEL_TOOL)
	$(call with_message,do model op $2)$$(UI_MODEL_TOOL) manip --model $$($1.ui-model.root) --format proj --op $3

endef

# }}}
# {{{ 实现：file序列帧导入

#$(call def-ui-model-file-effect,project,model)
define def-ui-model-file-effect-import

$1-model-import: $$($1.ui-model.root)/$$($1.ui-model.file-effect.$2.output)/$2.act

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.file-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/$$($1.ui-model.file-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/$$($1.ui-model.file-effect.$2.output))

.PHONY: $$($1.ui-model.file-effect.$2.input)/$2.tps
$$($1.ui-model.file-effect.$2.input)/$2.tps: $$(patsubst %,$$($1.ui-model.file-effect.$2.input)/%.png,$$($1.ui-model.file-effect.$2.pic-files))
	$(call with_message,generate tp project $2)$$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-tp-project.pl \
        --output-proj $$@ \
        --output-pic $$(call build-relative-path,$$($1.ui-model.root)/$$($1.ui-model.file-effect.$2.output).png,$$($1.ui-model.file-effect.$2.input)/$2.tps) \
        --output-plist $2.plist \
        $$(addprefix --input , $$(sort $$(notdir $$^)))

$$($1.ui-model.file-effect.$2.input)/$2.plist: $$($1.ui-model.file-effect.$2.input)/$2.tps
	$$(if $$(TEXTUREPACKER),$(call with_message,texture pack $2)'$$(TEXTUREPACKER)' --force-publish '$$(call cygwin-path-to-win,$$^)',echo "TEXTUREPACKER not set!")

$1.ui-model.file-effect.$2.frame-duration ?= 1
$1.ui-model.file-effect.$2.frame-position ?= center

$$($1.ui-model.root)/$$($1.ui-model.file-effect.$2.output)/$2.act: $$($1.ui-model.file-effect.$2.input)/$2.plist $$(UI_MODEL_TOOL)
	$(call with_message,import file action $2)$$(UI_MODEL_TOOL) cocos-effect-import \
        --model $$($1.ui-model.root) --format proj \
        --to-effect $$($1.ui-model.file-effect.$2.output) \
        --to-module $$($1.ui-model.file-effect.$2.output) \
        --frame-duration $$($1.ui-model.file-effect.$2.frame-duration) \
        --frame-position $$($1.ui-model.file-effect.$2.frame-position) \
        --frame-order $$($1.ui-model.file-effect.$2.frame-order) \
        --plist $$($1.ui-model.file-effect.$2.input)/$2.plist \
        --pic $$($1.ui-model.file-effect.$2.output).png

endef

# }}}
# {{{ 实现：dir序列帧合并导入

#$(call def-ui-model-dir-effect,project,model)
define def-ui-model-dir-effect-import

$1-model-import: $$($1.ui-model.root)/$$($1.ui-model.dir-effect.$2.output)/$2.act

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.dir-effect.$2.output))

.PHONY: $$($1.ui-model.dir-effect.$2.input)/$2.tps
$$($1.ui-model.dir-effect.$2.input)/$2.tps: $$(foreach m,$$(notdir $$(wildcard $$($1.ui-model.dir-effect.$2.input))/*),$$(wildcard $$($1.ui-model.dir-effect.$2.input)/$$m/*.png))
	$(call with_message,generate tp project $2)$$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-tp-project.pl \
        --output-proj $$@ \
        --output-pic $$(call build-relative-path,$$($1.ui-model.root)/$$($1.ui-model.dir-effect.$2.output).png,$$($1.ui-model.dir-effect.$2.input)/$2.tps) \
        --output-plist $2.plist \
        $$(addprefix --input , $$(sort $$(subst $$($1.ui-model.dir-effect.$2.input)/,,$$^)))

$$($1.ui-model.dir-effect.$2.input)/$2.plist: $$($1.ui-model.dir-effect.$2.input)/$2.tps
	$$(if $$(TEXTUREPACKER),$(call with_message,texture pack $2)'$$(TEXTUREPACKER)' --force-publish '$$(call cygwin-path-to-win,$$^)',echo "TEXTUREPACKER not set!")

$1.ui-model.dir-effect.$2.frame-duration ?= 1
$1.ui-model.dir-effect.$2.frame-position ?= center

$$($1.ui-model.root)/$$($1.ui-model.dir-effect.$2.output)/$2.act: $$($1.ui-model.dir-effect.$2.input)/$2.plist $$(UI_MODEL_TOOL)
	$(call with_message,import file action $2)$$(UI_MODEL_TOOL) cocos-effect-import \
        --model $$($1.ui-model.root) --format proj \
        --to-effect $$($1.ui-model.dir-effect.$2.output) \
        --to-module $$($1.ui-model.dir-effect.$2.output) \
        --frame-duration $$($1.ui-model.dir-effect.$2.frame-duration) \
        --frame-position $$($1.ui-model.dir-effect.$2.frame-position) \
        --frame-order $$($1.ui-model.dir-effect.$2.frame-order) \
        --plist $$($1.ui-model.dir-effect.$2.input)/$2.plist \
        --pic $$($1.ui-model.dir-effect.$2.output).png

endef

# }}}
# {{{ 实现：cocos图片模块

#$(call def-ui-model-cocos-module-import,product,model)
define def-ui-model-cocos-module-import

$$(call assert-not-null,$1.ui-model.cocos-module.$2.input)

$1-model-import: $$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).ibk

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).png) \
                 $$(dir $$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).ibk)

.PHONY: $$($1.ui-model.cocos-module.$2.input-base)$(strip $2).tps
$$($1.ui-model.cocos-module.$2.input-base)$(strip $2).tps: $$($1.ui-model.cocos-module.$(strip $2).input)
	$(call with_message,generate tp project $2)$$(CPDE_PERL) $$(CPDE_ROOT)/buildtools/tools/gen-tp-project.pl \
        --output-proj $$@ \
        --output-pic $$(call build-relative-path,$$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).png,$$@) \
        --output-plist $$(call build-relative-path,$$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).plist,$$@) \
        $$(addprefix --input , $$(foreach i,$$(sort $$^),$$(call build-relative-path,$$i,$$(dir $$@))))

$$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).plist: $$($1.ui-model.cocos-module.$2.input-base)$(strip $2).tps
	$$(if $$(TEXTUREPACKER),$(call with_message,texture pack $2)'$$(TEXTUREPACKER)' --force-publish '$$(call cygwin-path-to-win,$$^)',echo "TEXTUREPACKER not set!")

$$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).ibk: $$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).plist $$(UI_MODEL_TOOL)
	$(call with_message,import cocos module $2)$$(UI_MODEL_TOOL) cocos-module-import \
        --model $$($1.ui-model.root) --format proj \
        --to-module $$($1.ui-model.cocos-module.$2.output) \
        --plist $$($1.ui-model.root)/$$($1.ui-model.cocos-module.$2.output).plist \
        --pic $$($1.ui-model.cocos-module.$2.output).png

endef

# }}}
# {{{ 实现：cocos序列帧动画

#$(call def-ui-model-cocos-effect,model)
define def-ui-model-cocos-effect-import

$1-model-import: $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output).png \
                 $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output).ibk \
                 $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output).act

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output)) \
                 $$(dir $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output))

$$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output).png: $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.input).png
	cp $$< $$@

$1.ui-model.cocos-effect.$2.frame-duration ?= 1
$1.ui-model.cocos-effect.$2.frame-position ?= center
$1.ui-model.cocos-effect.$2.frame-order ?= native

.PHONY: $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output).act
$$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output).ibk $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.output).act: $$(wildcard $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.input).plist) $$(UI_MODEL_TOOL)
	$(call with_message,import cocos action $2)$$(UI_MODEL_TOOL) cocos-effect-import \
        --model $$($1.ui-model.root) --format proj \
        --to-effect $$($1.ui-model.cocos-effect.$2.output) \
        --to-module $$($1.ui-model.cocos-effect.$2.output) \
        --frame-duration $$($1.ui-model.cocos-effect.$2.frame-duration) \
        --frame-position $$($1.ui-model.cocos-effect.$2.frame-position) \
        --frame-order $$($1.ui-model.cocos-effect.$2.frame-order) \
        --plist $$($1.ui-model.root)/$$($1.ui-model.cocos-effect.$2.input).plist \
        --pic $$($1.ui-model.cocos-effect.$2.output).png

endef

# }}}
# {{{ 实现：cocos粒子动画

#$(call def-ui-model-cocos-particle,model)
define def-ui-model-cocos-particle-import

$1-model-import: $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.output).png \
                 $$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output).particle

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.output)) \
                 $$(dir $$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output))

$$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.output).png: $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.input).png
	cp $$< $$@

.PHONY: $$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output).particle
$$($1.ui-model.root)/Particle/$$($1.ui-model.cocos-particle.$2.output).particle: $$(wildcard $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.input).plist) $$(UI_MODEL_TOOL)
	$(call with_message,import cocos particle $2)$$(UI_MODEL_TOOL) cocos-particle-import \
        --model $$($1.ui-model.root) --format proj \
        --to-particle Particle/$$($1.ui-model.cocos-particle.$2.output) \
        --plist $$($1.ui-model.root)/$$($1.ui-model.cocos-particle.$2.input).plist \
        --pic $$($1.ui-model.cocos-particle.$2.output).png

endef

# }}}
# {{{ 实现：模块导入

#$(call def-ui-model-module-import,product,model)
define def-ui-model-module-import

$$(call assert-not-null,$1.ui-model.module.$2.input)

$1-model-import: $$($1.ui-model.root)/$$($1.ui-model.module.$2.output).ibk

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.module.$2.output).ibk)

.PHONY: $$($1.ui-model.root)/$$($1.ui-model.module.$2.output).ibk
$$($1.ui-model.root)/$$($1.ui-model.module.$2.output).ibk: $$($1.ui-model.module.$2.input) $$(UI_MODEL_TOOL)
	$(call with_message,import cocos module $2)$$(UI_MODEL_TOOL) module-import \
        --model $$($1.ui-model.root) --format proj \
        --to-module $$($1.ui-model.module.$2.output) \
        --import-base $$($1.ui-model.module.$2.input-base) \
        $$(addprefix --import , $$($1.ui-model.module.$2.input))

endef

# }}}
# {{{ 实现：spine动画

#$(call def-ui-model-spine-anim,model)
define def-ui-model-spine-anim-import

.PHONY: $1-model-import.spine
$1-model-import.spine: $$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-pic) \
                       $$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-spine)

$1-model-import: $1-model-import.spine

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-spine)) \
                 $$(dir $$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-pic)) \
                 $$(dir $$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-module)) \

$$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-pic): $$(patsubst %.json,%.png,$$($1.ui-model.spine-anim.$2.input))
	cp $$< $$@

#.PHONY: $$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-spine)
$$($1.ui-model.root)/$$($1.ui-model.spine-anim.$2.output-spine): $$($1.ui-model.spine-anim.$2.input) $$(UI_MODEL_TOOL)
	$(call with_message,import spine anim $2) \
	  cp $$< $$@ \
      && $$(UI_MODEL_TOOL) spine-anim-import \
        --model $$($1.ui-model.root) --format proj \
        --output-module $$($1.ui-model.spine-anim.$2.output-module) \
        --input-atlas $$(patsubst %.json,%.atlas,$$($1.ui-model.spine-anim.$2.input)) \
        --pic $$($1.ui-model.spine-anim.$2.output-pic)

endef

# }}}
# {{{ 实现：CrazyStorm弹幕导入

#$(call def-ui-model-crazystorm-barrage,model)
define def-ui-model-crazystorm-barrage-import

.PHONY: $1-model-import.barrage
$1-model-import: $1-model-import.barrage

$1-model-import.emitter: $$($1.ui-model.root)/$$($1.ui-model.crazystorm-barrage.$2.output)

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.crazystorm-barrage.$2.output)) \

$$($1.ui-model.root)/$$($1.ui-model.crazystorm-barrage.$2.output): $$($1.ui-model.crazystorm-barrage.$2.input) $$(UI_MODEL_TOOL)
	$(call with_message,import crazy-storm barrage $2) \
      $$(UI_MODEL_TOOL) crazystorm-emitter-import \
        --model $$($1.ui-model.root) --format proj \
        --output $$($1.ui-model.crazystorm-barrage.$2.output) \
        --input $$($1.ui-model.crazystorm-barrage.$2.input) \
        --bullets $$($1.ui-model.crazystorm-barrage.$2.bullets)

endef

# }}}
# {{{ 实现：Chipmunk从Sprite导入

#$(call def-ui-model-chipmunk-from-sprite,model)
define def-ui-model-chipmunk-from-sprite

.PHONY: $1-model-import.chipmunk

$1-model-import: $1-model-import.chipmunk

$1-model-import.chipmunk: $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-sprite.$2.output).chipmunk

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-sprite.$2.output))

.PHONY: $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-sprite.$2.output).chipmunk
$$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-sprite.$2.output).chipmunk: \
        $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-sprite.$2.input).frm \
        $$(UI_MODEL_TOOL)
	$(call with_message,import chipmunk $2 from sprite) \
      $$(UI_MODEL_TOOL) chipmunk-import-from-sprite \
        --model $$($1.ui-model.root) --format proj \
        --output $$($1.ui-model.chipmunk-from-sprite.$2.output) \
        --input $$($1.ui-model.chipmunk-from-sprite.$2.input)

endef

# }}}
# {{{ 实现：Chipmunk从RUBE导入

#$(call def-ui-model-chipmunk-from-rube,model)
define def-ui-model-chipmunk-from-rube

.PHONY: $1-model-import.chipmunk

$1-model-import: $1-model-import.chipmunk

$1-model-import.chipmunk: $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-rube.$2.output).chipmunk

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-rube.$2.output))

.PHONY: $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-rube.$2.output).chipmunk
$$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-rube.$2.output).chipmunk: \
        $$($1.ui-model.chipmunk-from-rube.$2.input).rube \
        $$(UI_MODEL_TOOL)
	$(call with_message,import chipmunk $2 from rube) \
      $$(UI_MODEL_TOOL) chipmunk-import-from-rube \
        --model $$($1.ui-model.root) --format proj \
        --output $$($1.ui-model.chipmunk-from-rube.$2.output) \
        --input $$($1.ui-model.chipmunk-from-rube.$2.input).rube \
        --ptm $$($1.ui-model.chipmunk-from-rube.$2.ptm)

endef

# }}}
# {{{ 实现：Chipmunk从Chipmunk官方支持的文件中导入

#$(call def-ui-model-chipmunk-from-chipmunk,model)
define def-ui-model-chipmunk-from-chipmunk

.PHONY: $1-model-import.chipmunk

$1-model-import: $1-model-import.chipmunk

$1-model-import.chipmunk: $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-chipmunk.$2.output).chipmunk

auto-build-dirs+=$$(dir $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-chipmunk.$2.output))

.PHONY: $$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-chipmunk.$2.output).chipmunk
$$($1.ui-model.root)/$$($1.ui-model.chipmunk-from-chipmunk.$2.output).chipmunk: \
        $$($1.ui-model.chipmunk-from-chipmunk.$2.input) \
        $$(UI_MODEL_TOOL)
	$(call with_message,import chipmunk $2 from chipmunk) \
      $$(UI_MODEL_TOOL) chipmunk-import-from-chipmunk \
        --model $$($1.ui-model.root) --format proj \
        --output $$($1.ui-model.chipmunk-from-chipmunk.$2.output) \
        --input $$($1.ui-model.chipmunk-from-chipmunk.$2.input) \
        $$(addprefix --adj-base , $$($1.ui-model.chipmunk-from-chipmunk.$2.adj-base)) \
        --ptm $$($1.ui-model.chipmunk-from-chipmunk.$2.ptm)

endef

# }}}
# {{{ 实现：TiledMap场景导入导出

#$(call def-ui-model-scene-import,model)
define def-ui-model-scene-import

.PHONY: $1-model-import.scene

$1-model-import.scene: $1-model-import.scene.$2

$1-model-import.scene.$2: $$(UI_MODEL_TOOL)
	$(call with_message,import scene $2) \
      $$(UI_MODEL_TOOL) tiledmap-scene-import \
         --model $$($1.ui-model.root) --format proj \
         --project $$($1.ui-model.scene-proj.$2.proj-path)

endef

#$(call def-ui-model-scene-export,model)
define def-ui-model-scene-export

.PHONY: $1-model-export.scene

$1-model-export.scene: $1-model-export.scene.$2

$1-model-export.scene.$2: $$(UI_MODEL_TOOL)
	$(call with_message,export scene $2) \
      $$(UI_MODEL_TOOL) tiledmap-scene-export \
         --model $$($1.ui-model.root) --format proj \
         --project $$($1.ui-model.scene-proj.$2.proj-path)

endef

# }}}
# {{{ 实现: TiledMap导出为图片

#$(call def-ui-model-tiledmap-to-pic,model)
define def-ui-model-tiledmap-to-pic

.PHONY: $1-model-export.tiledmap

$1-model-export: $1-model-export.tiledmap

$1-model-export.tiledmap: $$($1.ui-model.tiledmap-to-pic.$2.output)

auto-build-dirs+=$$(dir $$($1.ui-model.tiledmap-to-pic.$2.output))

.PHONY: $$($1.ui-model.tiledmap-to-pic.$2.output)
$$($1.ui-model.tiledmap-to-pic.$2.output): \
        $$($1.ui-model.root)/$$($1.ui-model.tiledmap-to-pic.$2.input).map \
        $$(UI_MODEL_TOOL)
	$(call with_message,export tiledmap $2 to pic) \
      $$(UI_MODEL_TOOL) tiledmap-scene-to-pic \
        --model $$($1.ui-model.root) --format proj \
        --output $$($1.ui-model.tiledmap-to-pic.$2.output) \
        --input $$($1.ui-model.tiledmap-to-pic.$2.input) \
        $$(addprefix --layer , $$($1.ui-model.tiledmap-to-pic.$2.layers)) \

endef

# }}}
# {{{ 实现：定义导入操作

define product-def-rule-ui-model-import

.PHONY: $1-model-repaire
$1-model-repaire: $$(UI_MODEL_TOOL)
	$(call with_message,repaire $1 ui model)$$(UI_MODEL_TOOL) repaire --model $$($1.ui-model.root) --format proj

.PHONY: $1-model-ops
$1-model-ops: $$(UI_MODEL_TOOL)
	@$(foreach op,$(patsubst %.yml,%,$(notdir $($1.ui-model.ops))),echo $1-model-op-$(op); )

.PHONY: $1-model-convert
$1-model-convert: $$(UI_MODEL_TOOL)
	$(call with_message,convert $1 ui model)$$(UI_MODEL_TOOL) convert --model $$($1.ui-model.root) --format proj --to $$(CPDE_OUTPUT_ROOT)/model-output --to-format proj \
       $$(addprefix --language ,$$($1.ui-model.language))

$(foreach op,$($1.ui-model.ops),$(call def-ui-model-tool-op,$1,$(patsubst %.yml,%,$(notdir $(op))),$(op)))

.PHONY: $1-model-import
$(foreach module,$($1.ui-model.dir-effect.modules),$(call def-ui-model-dir-effect-import,$1,$(module)))
$(foreach module,$($1.ui-model.file-effect.modules),$(call def-ui-model-file-effect-import,$1,$(module)))
$(foreach module,$($1.ui-model.cocos-module.modules),$(call def-ui-model-cocos-module-import,$1,$(module)))
$(foreach module,$($1.ui-model.cocos-effect.modules),$(call def-ui-model-cocos-effect-import,$1,$(module)))
$(foreach module,$($1.ui-model.cocos-particle.modules),$(call def-ui-model-cocos-particle-import,$1,$(module)))
$(foreach module,$($1.ui-model.module.modules),$(call def-ui-model-module-import,$1,$(module)))
$(foreach module,$($1.ui-model.spine-anim.modules),$(call def-ui-model-spine-anim-import,$1,$(module)))
$(foreach module,$($1.ui-model.crazystorm-barrage.modules),$(call def-ui-model-crazystorm-barrage-import,$1,$(module)))
$(foreach module,$($1.ui-model.crazystorm-bullet.modules),$(call def-ui-model-crazystorm-bullet-import,$1,$(module)))
$(foreach module,$($1.ui-model.chipmunk-from-sprite.modules),$(call def-ui-model-chipmunk-from-sprite,$1,$(module)))
$(foreach module,$($1.ui-model.chipmunk-from-rube.modules),$(call def-ui-model-chipmunk-from-rube,$1,$(module)))
$(foreach module,$($1.ui-model.chipmunk-from-chipmunk.modules),$(call def-ui-model-chipmunk-from-chipmunk,$1,$(module)))
$(foreach module,$($1.ui-model.scene-proj.modules),$(call def-ui-model-scene-import,$1,$(module)))

endef

# }}}
# {{{ 实现：定义导出操作

#$(call product-def-rule-ui-model-export,product,domain)
define product-def-rule-ui-model-export

$(call assert-not-null,$1.output)

.PHONY: $2.$1-model-export
$2.$1-model-export: $$(UI_MODEL_TOOL)
	$(call with_message,export bin files $2.$1)$$(UI_MODEL_TOOL) \
        convert --model=$$($1.ui-model.root) --format=proj --to=$$(if $$($1.ui-model.output),$$($1.ui-model.output),$$(CPDE_OUTPUT_ROOT)/$$($2.output)/$$($1.output)) --to-format=bin \
		$(if $(flex),--texture-limit-width=2048,$$(addprefix --texture-limit-width=,$$($1.ui-model.texture-limit-width))) \
        $(if $(flex),--texture-limit-height=2048,$$(addprefix --texture-limit-height=,$$($1.ui-model.texture-limit-height))) \
        $$(if $$($1.ui-model.texture-compress), \
              --texture-compress=$$($1.ui-model.texture-compress) \
              --accounts-tinypng=$$(CPDE_ROOT)/buildtools/accounts-tinypng.txt) \
        $(if $(flex),--pack-way=spack,) \
        $$(if $$(filter 1,$$($1.ui-model.full)), --full) \
        $$(addprefix --language=,$$($1.ui-model.language))

$(foreach module,$($1.ui-model.scene-proj.modules),$(call def-ui-model-scene-export,$1,$(module)))
$(foreach module,$($1.ui-model.tiledmap-to-pic.modules),$(call def-ui-model-tiledmap-to-pic,$1,$(module)))

endef

# }}}
# {{{ 实现：总入口

#main interface
define product-def-rule-ui-model

$(if $($1-rule-ui-model-defined),,$(eval $1-rule-ui-model-defined:=1)$(call product-def-rule-ui-model-import,$1))

$(call product-def-rule-ui-model-export,$1,$2)

endef

# }}}
