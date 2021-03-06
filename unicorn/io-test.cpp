#include "unicorn/core.hpp"
#include "unicorn/io.hpp"
#include "unicorn/utf.hpp"
#include "rs-core/file.hpp"
#include "rs-core/unit-test.hpp"
#include <algorithm>
#include <string>
#include <system_error>
#include <vector>

using namespace RS;
using namespace RS::Unicorn;
using namespace std::literals;

namespace {

    const RS::File testfile = "__test__";
    const RS::File nonesuch = "__no_such_file__";

}

void test_unicorn_io_file_reader() {

    Ustring s;
    Strings vec;
    Irange<FileReader> range;
    auto guard = scope_exit([=] { testfile.remove(); });

    TEST_THROW(range = read_lines(nonesuch.name()), std::system_error);
    TRY(range = read_lines(nonesuch.name(), IO::pretend));
    TEST_EQUAL(range_count(range), 0);

    TRY(testfile.save(
        "Last night I saw upon the stair\n"
        "A little man who wasn't there\n"
        "He wasn't there again today\n"
        "He must be from the NSA\n"
    ));
    TRY(range = read_lines(testfile.name()));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 4);
    TEST_EQUAL_RANGE(vec, (Strings{
        "Last night I saw upon the stair\n",
        "A little man who wasn't there\n",
        "He wasn't there again today\n",
        "He must be from the NSA\n",
    }));

    TRY(testfile.save(
        "Last night I saw upon the stair\r\n"
        "A little man who wasn't there\r\n"
        "He wasn't there again today\r\n"
        "He must be from the NSA\r\n"
    ));
    TRY(range = read_lines(testfile.name()));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 4);
    TEST_EQUAL_RANGE(vec, (Strings{
        "Last night I saw upon the stair\r\n",
        "A little man who wasn't there\r\n",
        "He wasn't there again today\r\n",
        "He must be from the NSA\r\n",
    }));

    TRY(testfile.save(
        "Last night I saw upon the stair\n"
        "A little man who wasn't there\n"
        "He wasn't there again today\n"
        "He must be from the NSA"
    ));
    TRY(range = read_lines(testfile.name()));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 4);
    TEST_EQUAL_RANGE(vec, (Strings{
        "Last night I saw upon the stair\n",
        "A little man who wasn't there\n",
        "He wasn't there again today\n",
        "He must be from the NSA",
    }));

    TRY(testfile.save(
        "Dollar\n"
        "\x80uro\n"
        "Pound\n"
    ));
    TRY(range = read_lines(testfile.name(), {}, "windows-1252"s));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 3);
    TEST_EQUAL_RANGE(vec, (Strings{
        u8"Dollar\n",
        u8"€uro\n",
        u8"Pound\n",
    }));

    TRY(testfile.save(
        "Dollar\n"
        "\x80uro\n"
        "Pound\n"
    ));
    TRY(range = read_lines(testfile.name()));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 3);
    TEST_EQUAL_RANGE(vec, (Strings{
        u8"Dollar\n",
        u8"\ufffduro\n",
        u8"Pound\n",
    }));

    TRY(testfile.save(
        "Dollar\n"
        "\x80uro\n"
        "Pound\n"
    ));
    TRY(range = read_lines(testfile.name(), Utf::throws));
    TEST_THROW(std::copy(range.begin(), range.end(), overwrite(vec)), EncodingError);

    TRY(testfile.save("Hello world\nGoodbye\n"));
    TRY(range = read_lines(testfile.name(), IO::bom));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\n", "Goodbye\n"}));

    TRY(testfile.save(u8"\ufeffHello world\nGoodbye\n"));
    TRY(range = read_lines(testfile.name(), IO::bom));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\n", "Goodbye\n"}));

    TRY(testfile.save("Hello world\nGoodbye\n"));
    TRY(range = read_lines(testfile.name(), IO::lf));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\n", "Goodbye\n"}));
    TRY(range = read_lines(testfile.name(), IO::crlf));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\r\n", "Goodbye\r\n"}));

    TRY(testfile.save("Hello world\r\nGoodbye\r\n"));
    TRY(range = read_lines(testfile.name(), IO::lf));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\n", "Goodbye\n"}));
    TRY(range = read_lines(testfile.name(), IO::crlf));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\r\n", "Goodbye\r\n"}));

    TRY(testfile.save("Hello world\rGoodbye\r"));
    TRY(range = read_lines(testfile.name(), IO::lf));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\n", "Goodbye\n"}));
    TRY(range = read_lines(testfile.name(), IO::crlf));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\r\n", "Goodbye\r\n"}));

    TRY(testfile.save(
        "\n"
        "Hello\n"
        "    \n"
        "    North South    \n"
        "    East West    \n"
        "    \n"
        "Goodbye\n"
        "\n"
    ));
    TRY(range = read_lines(testfile.name()));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 8);
    TEST_EQUAL_RANGE(vec, (Strings{
        "\n",
        "Hello\n",
        "    \n",
        "    North South    \n",
        "    East West    \n",
        "    \n",
        "Goodbye\n",
        "\n",
    }));
    TRY(range = read_lines(testfile.name(), IO::striplf));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 8);
    TEST_EQUAL_RANGE(vec, (Strings{
        "",
        "Hello",
        "    ",
        "    North South    ",
        "    East West    ",
        "    ",
        "Goodbye",
        "",
    }));
    TRY(range = read_lines(testfile.name(), IO::striptws));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 8);
    TEST_EQUAL_RANGE(vec, (Strings{
        "",
        "Hello",
        "",
        "    North South",
        "    East West",
        "",
        "Goodbye",
        "",
    }));
    TRY(range = read_lines(testfile.name(), IO::stripws));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 8);
    TEST_EQUAL_RANGE(vec, (Strings{
        "",
        "Hello",
        "",
        "North South",
        "East West",
        "",
        "Goodbye",
        "",
    }));
    TRY(range = read_lines(testfile.name(), IO::notempty));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 6);
    TEST_EQUAL_RANGE(vec, (Strings{
        "Hello\n",
        "    \n",
        "    North South    \n",
        "    East West    \n",
        "    \n",
        "Goodbye\n",
    }));
    TRY(range = read_lines(testfile.name(), IO::stripws | IO::notempty));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 4);
    TEST_EQUAL_RANGE(vec, (Strings{
        "Hello",
        "North South",
        "East West",
        "Goodbye",
    }));
    TRY(range = read_lines(testfile.name()));
    s.clear();
    for (auto fr = range.begin(); fr != range.end(); ++fr)
        TRY(s += dec(fr.line()) + ":"+ *fr);
    TEST_EQUAL(s,
        "1:\n"
        "2:Hello\n"
        "3:    \n"
        "4:    North South    \n"
        "5:    East West    \n"
        "6:    \n"
        "7:Goodbye\n"
        "8:\n"
    );
    TRY(range = read_lines(testfile.name(), IO::stripws | IO::notempty));
    s.clear();
    for (auto fr = range.begin(); fr != range.end(); ++fr)
        TRY(s += dec(fr.line()) + ":"+ *fr + "\n");
    TEST_EQUAL(s,
        "2:Hello\n"
        "4:North South\n"
        "5:East West\n"
        "7:Goodbye\n"
    );

    TRY(testfile.save("Hello world!!Goodbye!!"));
    TRY(range = read_lines(testfile.name(), {}, ""s, "!!"s));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world!!", "Goodbye!!"}));

    TRY(testfile.save("Hello world!!Goodbye!!"));
    TRY(range = read_lines(testfile.name(), IO::striplf, ""s, "!!"s));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world", "Goodbye"}));

    TRY(testfile.save("Hello world!!Goodbye!!"));
    TRY(range = read_lines(testfile.name(), IO::lf, ""s, "!!"s));
    TRY(std::copy(range.begin(), range.end(), overwrite(vec)));
    TEST_EQUAL(vec.size(), 2);
    TEST_EQUAL_RANGE(vec, (Strings{"Hello world\n", "Goodbye\n"}));

}

void test_unicorn_io_file_writer() {

    std::string s;
    Strings vec;
    FileWriter writer;
    auto guard = scope_exit([=] { testfile.remove(); });

    vec = {
        "Last night I saw upon the stair\n",
        "A little man who wasn't there\n",
        "He wasn't there again today\n",
        "He must be from the NSA\n",
    };
    TRY(writer = FileWriter(testfile.name()));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s,
        "Last night I saw upon the stair\n"
        "A little man who wasn't there\n"
        "He wasn't there again today\n"
        "He must be from the NSA\n"
    );

    vec = {"Hello world\r\n", "Goodbye\r\n"};
    TRY(writer = FileWriter(testfile.name()));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\r\nGoodbye\r\n");

    vec = {u8"Hello €urope\n", "Goodbye\n"};
    TRY(writer = FileWriter(testfile.name()));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, u8"Hello €urope\nGoodbye\n");

    // Error detection is not reliable on Windows
    #ifdef _XOPEN_SOURCE
        vec = {u8"Hello €urope\n", "Goodbye\n"};
        TRY(writer = FileWriter(testfile.name(), Utf::throws, "ascii"s));
        TEST_THROW(std::copy(vec.begin(), vec.end(), writer), EncodingError);
    #endif

    vec = {"Hello world\n", "Goodbye\n"};
    TRY(writer = FileWriter(testfile.name(), IO::bom));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, u8"\ufeffHello world\nGoodbye\n");

    vec = {u8"\ufeffHello world\n", "Goodbye\n"};
    TRY(writer = FileWriter(testfile.name(), IO::bom));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, u8"\ufeffHello world\nGoodbye\n");

    TRY(writer = FileWriter(testfile.name()));
    TRY(*writer++ = "Hello world\n");
    TRY(writer = FileWriter(testfile.name(), IO::append));
    TRY(*writer++ = "Goodbye\n");
    TRY(writer = FileWriter());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\nGoodbye\n");

    TEST_THROW(FileWriter(testfile.name(), IO::protect), std::system_error);

    vec = {"Hello world\n", "Goodbye\n"};
    TRY(writer = FileWriter(testfile.name(), IO::lf));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\nGoodbye\n");
    TRY(writer = FileWriter(testfile.name(), IO::crlf));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\r\nGoodbye\r\n");

    vec = {"Hello world\r\n", "Goodbye\r\n"};
    TRY(writer = FileWriter(testfile.name(), IO::lf));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\nGoodbye\n");
    TRY(writer = FileWriter(testfile.name(), IO::crlf));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\r\nGoodbye\r\n");

    vec = {"Hello world\r", "Goodbye\r"};
    TRY(writer = FileWriter(testfile.name(), IO::lf));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\nGoodbye\n");
    TRY(writer = FileWriter(testfile.name(), IO::crlf));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\r\nGoodbye\r\n");

    vec = {"Hello world", "Goodbye"};
    TRY(writer = FileWriter(testfile.name(), IO::writeline));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\nGoodbye\n");

    vec = {"Hello world", "Goodbye"};
    TRY(writer = FileWriter(testfile.name(), IO::crlf | IO::writeline));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "Hello world\r\nGoodbye\r\n");

    vec = {"North", "South\n", "East", "West\r\n"};
    TRY(writer = FileWriter(testfile.name(), IO::autoline));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "North\nSouth\nEast\nWest\r\n");

    vec = {"North", "South\n", "East", "West\r\n"};
    TRY(writer = FileWriter(testfile.name(), IO::lf | IO::autoline));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "North\nSouth\nEast\nWest\n");

    vec = {"North", "South\n", "East", "West\r\n"};
    TRY(writer = FileWriter(testfile.name(), IO::crlf | IO::autoline));
    TRY(std::copy(vec.begin(), vec.end(), writer));
    TRY(writer.flush());
    TRY(s = testfile.load());
    TEST_EQUAL(s, "North\r\nSouth\r\nEast\r\nWest\r\n");

}
