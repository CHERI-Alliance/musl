import os
import sys

def report_testcase(_line, _expected_failures):
    if not (_line.startswith("FAIL") or _line.startswith("PASS")):
        return

    tname = _line.replace("PASS ", "").replace("FAIL ", "")

    def test_file_ext(ext, remove_static=True):
        return os.path.join(os.getcwd(),
                            tname.replace(".exe", ext).
                                  replace("-static", "" if remove_static else "-static"))

    def ext_exists(ext, remove_static=True):
        return os.path.isfile(test_file_ext(ext, remove_static))

    print("    <testcase name=\"{}\">".format(tname))

    if _line.startswith("FAIL"):
        if tname in _expected_failures:
            print("      <skipped/>")

        if ext_exists(".exe", remove_static=False):
            print("      <failure><![CDATA[")
            with open(test_file_ext(".err")) as _f:
                print(_f.read())
            print("      ]]></failure>")

        elif ext_exists(".exe.ld.err"):
            print("      <error type=\"Linker error\"><![CDATA[")
            with open(test_file_ext(".exe.ld.err")) as _f:
                print(_f.read())
            print("      ]]></error>")

        elif ext_exists(".o.err"):
            print("      <error type=\"Compiler error\"><![CDATA[")
            with open(test_file_ext(".o.err")) as _f:
                print(_f.read())
            print("      ]]></error>")

        else:
            print("      <error type=\"Unknown error\"></error>")

    print("    </testcase>")


if __name__ == "__main__":
    testsuite_name = sys.argv[1]
    expected_failures = []
    if len(sys.argv) > 2:
        expected_failures_file = sys.argv[2]
        with open(expected_failures_file) as f:
            expected_failures = f.read().split("\n")

    print("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>")
    print("<testsuites>")
    print("  <testsuite name=\"{}\">".format(testsuite_name))

    with open(os.path.join(os.getcwd(), "src", "REPORT")) as f:
        for line in f.read().split("\n"):
            report_testcase(line, expected_failures)

    print("  </testsuite>")
    print("</testsuites>")
