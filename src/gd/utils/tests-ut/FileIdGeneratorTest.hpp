#ifndef GDPP_UTILS_TEST_FILEIDGENERATOR_H
#define GDPP_UTILS_TEST_FILEIDGENERATOR_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_file.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/utils/id_generator.h"
#include "gd/utils/id_generator_file.h"

typedef LOKI_TYPELIST_3(
    utils::testenv::with_em
    , utils::testenv::with_file
    , gd::app::testenv::with_app
    ) FileIdGeneratorTestBase;

class FileIdGeneratorTest : public testenv::fixture<FileIdGeneratorTestBase> {
public:
    virtual void SetUp();

    void write_in_file(const char * value, const char * id_name);

    void set_load_from_dir(void);

    gd_id_generator_t id_generator(void);
    gd_id_file_generator_t id_file_generator(void);
};

#endif
