#include <gtest/gtest.h>
#include <sese/db/DriverManager.h>

#include "Config.h"

class TestSqliteMetadata : public testing::Test {
protected:
    static sese::db::DriverInstance::Ptr instance;

    static void SetUpTestSuite() {
        instance = sese::db::DriverManager::getInstance(sese::db::DatabaseType::SQLITE, SQLITE_CONNECTION_STRING);
        ASSERT_NE(nullptr, instance);
    }
};

sese::db::DriverInstance::Ptr TestSqliteMetadata::instance;

TEST_F(TestSqliteMetadata, CheckType) {
    auto stmt = instance->createStatement("select * from tb_metadata;");
    ASSERT_NE(nullptr, stmt);
    {
        sese::db::MetadataType type;
        ASSERT_EQ(true, stmt->getColumnType(0, type));
        EXPECT_EQ(sese::db::MetadataType::INTEGER, type);
    }
    {
        sese::db::MetadataType type;
        ASSERT_EQ(true, stmt->getColumnType(1, type));
        EXPECT_EQ(sese::db::MetadataType::TEXT, type);
    }
    {
        sese::db::MetadataType type;
        ASSERT_EQ(true, stmt->getColumnType(2, type));
        EXPECT_EQ(sese::db::MetadataType::FLOAT, type);
    }
    {
        sese::db::MetadataType type;
        ASSERT_EQ(true, stmt->getColumnType(3, type));
        EXPECT_EQ(sese::db::MetadataType::UNKNOWN, type);
    }
    {
        // 越界
        sese::db::MetadataType type;
        ASSERT_EQ(false, stmt->getColumnType(4, type));
    }
}

TEST_F(TestSqliteMetadata, CheckSize) {
    auto stmt = instance->createStatement("select * from tb_metadata;");
    ASSERT_NE(nullptr, stmt);

    EXPECT_EQ(stmt->getColumnSize(0), 0);
    EXPECT_EQ(stmt->getColumnSize(1), 16);
    EXPECT_EQ(stmt->getColumnSize(2), 0);
    EXPECT_EQ(stmt->getColumnSize(3), 32);
    EXPECT_EQ(stmt->getColumnSize(4), -1);
}