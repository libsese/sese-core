#include "sese/config/yaml/YamlUtil.h"
#include "sese/config/yaml/Marco.h"
#include "sese/io/InputBufferWrapper.h"
#include "sese/io/ConsoleOutputStream.h"

#include "gtest/gtest.h"

constexpr char STR1[]{
        "root:\n"
        "  mapping:\n"
        "    str 1: \"Hello:-|str1\"\n"
        "    \"str\'2\": 'Hello str2'\n"
        "    str\"3: \"Hello\\\"str3\"\n"
        "    str|4: 'Hello:-|\"str4'\n"
        "    str5: Hello str5\n"
        "    bool1: true\n"
        "    bool2: yes\n"
        "    bool3: or\n"
        "    int1: 114514\n"
        "    float1: 3.14\n"
        "    float2: 3.14e+5\n"
        "    null1: ~\n"
        "    null2: null\n"
        "    null3: Null\n"
        "    null4:\n"
        "\n"
        "  sequence:\n"
        "    - element1\n"
        "    - element2\n"
        "    -\n"
        "    - ~\n"
        "    - mapping1:\n"
        "      bool1: false\n"
        "      bool2: no\n"
        "    - mapping2:\n"
        "      bool1: False\n"
        "      bool2: No\n"
        "sub:\n"
        "  str5: 'Hello str5'"
};

TEST(TestYaml, Tokenizer_0) {
    auto input = sese::io::InputBufferWrapper(STR1, sizeof(STR1) - 1);

    while (true) {
        decltype(auto) tuple = sese::yaml::YamlUtil::getLine(&input);
        if (std::get<1>(tuple).empty()) {
            break;
        }
        auto tokens = sese::yaml::YamlUtil::tokenizer(std::get<1>(tuple));
        printf("%d [", (int) std::get<0>(tuple));
        bool first = true;
        for (decltype(auto) token: tokens) {
            if (first) {
                first = false;
            } else {
                printf(", ");
            }
            printf("\"%s\"", token.c_str());
        }
        printf("]\n");
    }
}

TEST(TestYaml, Deserialize_0) {
    auto input = sese::io::InputBufferWrapper(STR1, sizeof(STR1) - 1);
    auto object = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(object, nullptr);
    auto obj = std::dynamic_pointer_cast<sese::yaml::ObjectData>(object);

    auto sub_obj = std::dynamic_pointer_cast<sese::yaml::ObjectData>(obj->get("sub"));
    auto str5_obj = std::dynamic_pointer_cast<sese::yaml::BasicData>(sub_obj->get("str5"));
    EXPECT_EQ(str5_obj->getDataAs<std::string>("undef"), "Hello str5");

    auto root_obj = std::dynamic_pointer_cast<sese::yaml::ObjectData>(obj->get("root"));
    auto mapping_obj = std::dynamic_pointer_cast<sese::yaml::ObjectData>(root_obj->get("mapping"));

    auto str2_obj = std::dynamic_pointer_cast<sese::yaml::BasicData>(mapping_obj->get("str'2"));
    EXPECT_EQ(str2_obj->getDataAs<std::string>("undef"), "Hello str2");

    auto bool2_obj = std::dynamic_pointer_cast<sese::yaml::BasicData>(mapping_obj->get("bool2"));
    EXPECT_EQ(bool2_obj->getDataAs<bool>(false), true);

    auto bool3_obj = std::dynamic_pointer_cast<sese::yaml::BasicData>(mapping_obj->get("bool3"));
    EXPECT_EQ(bool3_obj->getDataAs<bool>(true), false);

    auto float2_obj = std::dynamic_pointer_cast<sese::yaml::BasicData>(mapping_obj->get("float2"));
    EXPECT_EQ(float2_obj->getDataAs<double>(0.0), 3.14e5);

    auto int2_obj = std::dynamic_pointer_cast<sese::yaml::BasicData>(mapping_obj->get("int1"));
    EXPECT_EQ(int2_obj->getDataAs<int64_t>(0), 114514);

    auto array_obj = std::dynamic_pointer_cast<sese::yaml::ArrayData>(root_obj->get("sequence"));
    auto element1_obj = std::dynamic_pointer_cast<sese::yaml::BasicData>(*array_obj->begin());
    EXPECT_EQ(element1_obj->getDataAs<std::string>("undef"), "element1");

    /// 数据为 null
    SESE_YAML_GET_STRING(s0, mapping_obj, "null1", "undef");
    EXPECT_EQ(s0, "undef");
    SESE_YAML_GET_BOOLEAN(b0, mapping_obj, "null1", false);
    EXPECT_EQ(b0, false);
    SESE_YAML_GET_DOUBLE(d0, mapping_obj, "null1", 0.0);
    EXPECT_EQ(d0, 0.0);
    SESE_YAML_GET_INTEGER(i0, mapping_obj, "null1", 0);
    EXPECT_EQ(i0, 0);

    /// 数据不存在
    SESE_YAML_GET_STRING(s1, mapping_obj, "A", "undef");
    EXPECT_EQ(s1, "undef");
    SESE_YAML_GET_BOOLEAN(b1, mapping_obj, "A", false);
    EXPECT_EQ(b1, false);
    SESE_YAML_GET_DOUBLE(d1, mapping_obj, "A", 0.0);
    EXPECT_EQ(d1, 0.0);
    SESE_YAML_GET_INTEGER(i1, mapping_obj, "A", 0);
    EXPECT_EQ(i1, 0);
}

/// 字符串转义
TEST(TestYaml, Deserialize_1) {
    constexpr char STR[]{R"(str: "\n\r\'\"\a")"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(OBJECT, nullptr);
}

/// createObject
/// 不存在下一行且当前元素为 null
TEST(TestYaml, Deserialize_2) {
    constexpr char STR[]{"root:\n"
                         "  null: "};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(OBJECT, nullptr);
}

/// createObject
/// 当前行元素是 null，下一个元素依然是本对象所属
TEST(TestYaml, Deserialize_3) {
    constexpr char STR[]{"root:\n"
                         "  a:\n"
                         "  b: str"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(OBJECT, nullptr);
}

/// createArray
/// 不存在下一行，跳过该对象
TEST(TestYaml, Deserialize_4) {
    constexpr char STR[]{"root:\n"
                         "  - ele:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(OBJECT, nullptr);
}

/// createArray
/// 数组嵌套数组
TEST(TestYaml, Deserialize_5) {
    constexpr char STR[]{"- root:\n"
                         "  - ele:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(OBJECT, nullptr);
}

TEST(TestYaml, Deserialize_6) {
    constexpr char STR[]{"- Hello\n"
                         "- World\n"
                         "- null\n"
                         "- ~"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(OBJECT, nullptr);
}

/// 对象无子元素
TEST(TestYaml, Deserialize_7) {
    constexpr char STR[]{"root:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 5);
    ASSERT_NE(OBJECT, nullptr);
}

TEST(TestYaml, Serialize_0) {
    sese::io::ConsoleOutputStream output;

    const auto ROOT = std::make_shared<sese::yaml::ObjectData>();

    const auto OBJECT = std::make_shared<sese::yaml::ObjectData>();
    ROOT->set("object", OBJECT);
    // auto name = std::make_shared<sese::yaml::BasicData>();
    // name->setDataAs<std::string>("sese-core");
    // object->set("name", name);
    // auto version = std::make_shared<sese::yaml::BasicData>();
    // version->setDataAs<std::string>("0.6.3");
    // object->set("version", version);
    SESE_YAML_SET_STRING(OBJECT, "name", "sese-core");
    SESE_YAML_SET_STRING(OBJECT, "version", "0.6.3");
    SESE_YAML_SET_BOOLEAN(OBJECT, "bool1", false);
    SESE_YAML_SET_BOOLEAN(OBJECT, "bool2", true);
    SESE_YAML_SET_DOUBLE(OBJECT, "double", 3.14);
    SESE_YAML_SET_NULL(OBJECT, "nullable");

    const auto ARRAY = std::make_shared<sese::yaml::ArrayData>();
    ROOT->set("array", ARRAY);
    // auto element0 = std::make_shared<sese::yaml::BasicData>();
    // element0->setDataAs<int64_t>(1919810);
    // array->push(element0);
    // auto element1 = std::make_shared<sese::yaml::BasicData>();
    // element1->setDataAs<double>(3.1415926);
    // array->push(element1);
    SESE_YAML_PUT_INTEGER(ARRAY, 1919810);
    SESE_YAML_PUT_DOUBLE(ARRAY, 3.1415926);

    auto complex = std::make_shared<sese::yaml::ObjectData>();
    ARRAY->push(complex);
    // auto real = std::make_shared<sese::yaml::BasicData>();
    // real->setDataAs<int64_t>(1);
    // complex->set("real", real);
    // auto image = std::make_shared<sese::yaml::BasicData>();
    // image->setDataAs<int64_t>(-3);
    // complex->set("image", image);
    SESE_YAML_SET_INTEGER(complex, "real", 1);
    SESE_YAML_SET_INTEGER(complex, "image", -3);

    sese::yaml::YamlUtil::serialize(ROOT, &output);
}

TEST(TestYaml, Serialize_1) {
    sese::io::ConsoleOutputStream output;

    auto array = std::make_shared<sese::yaml::ArrayData>();
    SESE_YAML_PUT_STRING(array, "value0");
    SESE_YAML_PUT_BOOLEAN(array, true);
    SESE_YAML_PUT_BOOLEAN(array, false);
    SESE_YAML_PUT_INTEGER(array, 114514);
    SESE_YAML_PUT_INTEGER(array, 1919810);
    SESE_YAML_PUT_DOUBLE(array, 3.14);
    SESE_YAML_PUT_DOUBLE(array, 0.15926e-2);
    SESE_YAML_PUT_NULL(array);

    sese::yaml::YamlUtil::serialize(array, &output);
}

TEST(TestYaml, Serialize_2) {
    sese::io::ConsoleOutputStream output;
    auto array = std::make_shared<sese::yaml::ArrayData>();
    auto subarray = std::make_shared<sese::yaml::ArrayData>();
    array->push(subarray);

    SESE_YAML_PUT_STRING(subarray, "value0");
    SESE_YAML_PUT_BOOLEAN(subarray, true);
    SESE_YAML_PUT_BOOLEAN(subarray, false);
    SESE_YAML_PUT_INTEGER(subarray, 114514);
    SESE_YAML_PUT_INTEGER(subarray, 1919810);
    SESE_YAML_PUT_DOUBLE(subarray, 3.14);
    SESE_YAML_PUT_DOUBLE(subarray, 0.15926e-2);
    SESE_YAML_PUT_NULL(subarray);

    sese::yaml::YamlUtil::serialize(array, &output);
}

TEST(TestYaml, Serialize_3) {
    sese::io::ConsoleOutputStream output;
    auto data = std::make_shared<sese::yaml::BasicData>();
    data->setDataAs<std::string>("data");
    sese::yaml::YamlUtil::serialize(data, &output);
}

#include <sese/config/Yaml.h>

TEST(TestYaml, Value) {
    auto input = sese::io::InputBufferWrapper(STR1, sizeof(STR1) - 1);
    const auto VALUE = sese::Yaml::parse(&input, 5);
    ASSERT_FALSE(VALUE.isNull());
    const auto CONTENT = VALUE.toString(5);
    puts(CONTENT.c_str());
}

/// 数组嵌套数组超出深度限制
TEST(TestYaml, Error_0) {
    constexpr char STR[]{"- root:\n"
                         "    - ele:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 1);
    ASSERT_EQ(OBJECT, nullptr);
}

/// 数组嵌套对象超出深度限制
TEST(TestYaml, Error_1) {
    constexpr char STR[]{"- root:\n"
                         "    ele:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 1);
    ASSERT_EQ(OBJECT, nullptr);
}

/// 对象嵌套数组超出深度限制
TEST(TestYaml, Error_3) {
    constexpr char STR[]{"root:\n"
                         "  - ele:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 1);
    ASSERT_EQ(OBJECT, nullptr);
}

/// 对象嵌套对象超出深度限制
TEST(TestYaml, Error_4) {
    constexpr char STR[]{"root:\n"
                     "   ele:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 1);
    ASSERT_EQ(OBJECT, nullptr);
}

TEST(TestYaml, Error_5) {
    constexpr char STR[]{"root1:\n"
                     ":root2:"};
    auto input = sese::io::InputBufferWrapper(STR, sizeof(STR) - 1);
    const auto OBJECT = sese::yaml::YamlUtil::deserialize(&input, 1);
    ASSERT_EQ(OBJECT, nullptr);
}