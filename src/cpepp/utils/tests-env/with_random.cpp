#include "cpepp/utils/tests-env/with_random.hpp"

namespace Cpe { namespace Utils { namespace testenv {

with_random::RandomMock &
with_random::t_random_mock(void) {
    return m_random_mock;
}

void with_random::t_random_expect_gen_with_arg(uint32_t arg, uint32_t r) {
    EXPECT_CALL(m_random_mock, generate(::testing::Eq(arg)))
        .Times(1)
        .WillOnce(::testing::Return(r));
}

void with_random::t_random_expect_gen_with_arg(uint32_t arg, uint32_t r1, uint32_t r2) {
    EXPECT_CALL(m_random_mock, generate(::testing::Eq(arg)))
        .Times(2)
        .WillOnce(::testing::Return(r1))
        .WillOnce(::testing::Return(r2));
}

void with_random::t_random_expect_gen_with_arg(uint32_t arg, uint32_t r1, uint32_t r2, uint32_t r3) {
    EXPECT_CALL(m_random_mock, generate(::testing::Eq(arg)))
        .Times(3)
        .WillOnce(::testing::Return(r1))
        .WillOnce(::testing::Return(r2))
        .WillOnce(::testing::Return(r3));
}

void with_random::t_random_expect_gen(uint32_t r) {
    EXPECT_CALL(m_random_mock, generate(::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(r));
}

void with_random::t_random_expect_gen(uint32_t r1, uint32_t r2) {
    EXPECT_CALL(m_random_mock, generate(::testing::_))
        .Times(2)
        .WillOnce(::testing::Return(r1))
        .WillOnce(::testing::Return(r2))
        ;
}

void with_random::t_random_expect_gen(uint32_t r1, uint32_t r2, uint32_t r3) {
    EXPECT_CALL(m_random_mock, generate(::testing::_))
        .Times(3)
        .WillOnce(::testing::Return(r1))
        .WillOnce(::testing::Return(r2))
        .WillOnce(::testing::Return(r3))
        ;
}

void with_random::t_random_expect_default( uint32_t r, uint32_t nTime){
	ON_CALL( m_random_mock,generate(::testing::_))
		.WillByDefault( ::testing::Return( r ) );

	EXPECT_CALL(m_random_mock, generate(::testing::_))
		.Times(::testing::AtLeast(nTime));
}

void with_random::t_random_not_care(){
	EXPECT_CALL(m_random_mock, generate(::testing::_))
		.Times(::testing::AtLeast(0));
}

}}}
