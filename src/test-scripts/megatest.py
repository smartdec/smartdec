# -*- coding: utf-8 -*-

# SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
# Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
# Alexander Fokin, Sergey Levin, Leonid Tsvetkov
#
# This file is part of SmartDec decompiler.
#
# SmartDec decompiler is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SmartDec decompiler is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.


import subprocess, threading, sys, time, string, itertools, re

class ExecutionTimeout(Exception):
    def __str__(self):
        return "Execution timeout"

def execute(cmdline, timeout=None, **kwargs):
    class Launcher(object):
        def __init__(self, cmdline, **kwargs):
            self.cmdline = cmdline
            self.kwargs = kwargs
            self.process = None
            self.exception = None

        def __call__(self):
            try:
                self.process = subprocess.Popen(self.cmdline, **self.kwargs)
                self.process.communicate()
            except OSError as e:
                self.exception = e

    launcher = Launcher(cmdline, **kwargs)

    thread = threading.Thread(target=launcher)
    thread.start()
    thread.join(timeout)

    if thread.is_alive():
        launcher.process.terminate()
        thread.join()
        raise ExecutionTimeout()

    if launcher.exception != None:
        raise launcher.exception

    return launcher.process.returncode

class TestFailed(Exception):
    def __init__(self, reason):
        super(Exception, self).__init__()
        self.reason = reason

    def __str__(self):
        return "Test failed: " + str(self.reason)

class Test(object):
    def __init__(self, name):
        super(Test, self).__init__()
        self.name = name

    def __call__(self):
        try:
            self.run()
        except TestFailed:
            raise
        except:
            self.fail("caught an exception: %s" % str(sys.exc_info()[1]))

    def fail(self, reason):
        raise TestFailed(reason)

class ExecutionTest(Test):
    def __init__(self, name, cmdline, stdoutFile=None, stderrFile=None, timeout=None, expectedExitCode=None):
        super(ExecutionTest, self).__init__(name)
        self.cmdline = cmdline
        self.stdoutFile = stdoutFile
        self.stderrFile = stderrFile
        self.timeout = timeout
        self.expectedExitCode = expectedExitCode

    def run(self):
        stdout = None
        stderr = None

        try:
            if (self.stdoutFile != None):
                stdout = open(self.stdoutFile, "w")

            if (self.stderrFile != None):
                stderr = open(self.stderrFile, "w")

            exitCode = execute(self.cmdline, timeout=self.timeout, stdout=stdout, stderr=stderr)
        finally:
            if stdout != None:
                stdout.close()
            if stderr != None:
                stderr.close()

        if self.expectedExitCode != None and exitCode != self.expectedExitCode:
            self.fail("exit code is %d, expected %d" % (exitCode, self.expectedExitCode))

class TextFileComparisonTest(Test):
    def __init__(self, name, file1, file2):
        super(TextFileComparisonTest, self).__init__(name)
        self.file1 = file1
        self.file2 = file2

    def run(self):
        def normalizeEndings(s):
            return s.replace(b"\r\n", b"\n").replace(b"\r", b"\n")

        fd1 = None
        fd2 = None
        try:
            fd1 = open(self.file1, "rb")
            fd2 = open(self.file2, "rb")
            if normalizeEndings(fd1.read()) != normalizeEndings(fd2.read()):
                self.fail("files differ")
        finally:
            if fd1 != None:
                fd1.close()
            if fd2 != None:
                fd2.close()

class RegexpFileTest(Test):
    def __init__(self, name, file, regexp):
        super(RegexpFileTest, self).__init__(name)
        self.file = file
        self.regexp = regexp

    def run(self):
        fd = None
        try:
            fd = open(self.file, "rb")
            if not re.search(self.regexp, fd.read()):
                self.fail("file contents does not match the regular expression: %s" % self.regexp)
        finally:
            if fd != None:
                fd.close()

def runTests(tests, out=sys.stdout):
    failedTests = []

    maxIndexLength = len(str(len(tests)))

    maxNameLength = 0
    for test in tests:
        maxNameLength = max(maxNameLength, len(test.name))

    startTime = time.time()

    for i, test in zip(itertools.count(1), tests):
        out.write("Test %%%dd of %%d: %%s %%s " % maxIndexLength %
            (i, len(tests), test.name, '.' * (maxNameLength - len(test.name) + 3)))
        out.flush()

        testStartTime = time.time()

        try:
            test()
            sys.stdout.write("   Passed")
        except TestFailed as e:
            sys.stdout.write("***Failed")
            failedTests.append((test, e.reason))

        testEndTime = time.time()

        out.write("  %7.2f sec\n" % (testEndTime - testStartTime))


    endTime = time.time()

    if tests:
        percentage = (len(tests) - len(failedTests)) * 100 / len(tests)
    else:
        percentage = 100

    out.write("\n%d%% tests passed, %d tests failed out of %d.\n" % (percentage, len(failedTests), len(tests)))

    out.write("\nTotal testing time: %0.2f sec.\n" % (endTime - startTime))

    if failedTests:
        out.write("\nThe following tests have FAILED:\n")
        for test, reason in failedTests:
            out.write("    %%-%ds    (%%s)\n" % maxNameLength % (test.name, reason))

    return len(failedTests)

# vim:set et sts=4 sw=4: