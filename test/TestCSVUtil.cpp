#include "sese/config/CSVReader.h"
#include "sese/config/CSVWriter.h"
#include "sese/io/InputBufferWrapper.h"
#include "sese/io/ConsoleOutputStream.h"
#include "gtest/gtest.h"

TEST(TestCSVUtil, Reader) {
    const char BUFFER[] = "A1,B1,C1,D1,E1,\r\n"
                          "A2,B2,C2,D2,E2,F2\r\n"
                          "A3,,C3,\"D3\",,\n"
                          "A4,B4,C4,D4,E4,";
    auto in = sese::io::InputBufferWrapper(BUFFER, sizeof(BUFFER) - 1);
    sese::CSVReader reader(&in);
    sese::CSVReader::Row row;
    do {
        row = reader.read();
        bool first = true;
        for (const auto &col: row) {
            if (first) {
                first = false;
                printf("%s", col.empty() ? "Null" : col.c_str());
            } else {
                printf(",%s", col.empty() ? "Null" : col.c_str());
            }
        }
        puts("");
    } while (!row.empty());
}

TEST(TestCSVUtil, Writer) {
    sese::io::ConsoleOutputStream console;
    sese::CSVWriter writer1(&console, '|');
    writer1.write({"A1", "B1", "C1", ""});
    writer1.write({"A2", "B2", "C2"});
    writer1.write({"A3", "", "C3"});

    sese::CSVWriter writer2(&console, '^', false);
    writer2.write({"A1", "B1", "C1", ""});
    writer2.write({"A2", "B2", "C2"});
    writer2.write({"A3", "", "C3"});
}