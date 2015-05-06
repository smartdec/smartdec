#!/usr/bin/env python

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


import megatest, os, glob, shutil, sys, itertools, argparse

def findExecutable(root, names):
    for dirpath, dirnames, filenames in os.walk(root):
        for filename in filenames:
            if filename in names:
                path = os.path.join(dirpath, filename)
                if os.access(path, os.X_OK):
                    return path
    return None

def recreateDirectory(path):
    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)

class Environment(object):
    def __init__(self, buildDirectory=None, scratchDirectory=None, decompiler=None, timeout=None):
        self.projectRoot      = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        self.buildDirectory   = os.path.abspath(buildDirectory or os.path.join(self.projectRoot, "build"))
        self.scratchDirectory = os.path.abspath(scratchDirectory or os.path.join(self.buildDirectory, "scratch"))
        self.decompiler       = os.path.abspath(decompiler or findExecutable(self.buildDirectory, ["nocode", "nocode.exe"]))
        self.timeout          = timeout
        self.expectedExitCode = 0
        self.tests            = []

    def dump(self, out = sys.stdout):
        out.write("Project root: %s\n" % self.projectRoot)
        out.write("Build directory: %s\n" % self.buildDirectory)
        out.write("Scratch directory: %s\n" % self.scratchDirectory)
        out.write("Decompiler: %s\n" % self.decompiler)

    def addTest(self, test):
        self.tests.append(test)
        return test

    def runTests(self):
        recreateDirectory(self.scratchDirectory)
        return megatest.runTests(self.tests)

    def getStreamFileName(self, testName, stream):
        return os.path.join(self.scratchDirectory, "%s.%s" % (testName, stream))

    def getCookieFileName(self, filename, cookie):
        return os.path.join(os.path.dirname(filename), "cookies", os.path.basename(filename) + "." + cookie)

    def getCookieValue(self, filename, cookie, defaultValue=None):
        try:
            file = open(self.getCookieFileName(filename, cookie), "rb")
        except IOError:
            return defaultValue
        try:
            return file.read()
        except IOError:
            return defaultValue
        finally:
            file.close()

    def getExpectedExitCode(self, filename):
        return int(self.getCookieValue(filename, "exitCode", self.expectedExitCode))

    def getTimeout(self, filename):
        return int(self.getCookieValue(filename, "timeout", self.timeout))

    def getRegularExpressions(self, filename, stream):
        return filter(
            lambda line: not line.startswith(b"#") and line.strip(),
            self.getCookieValue(filename, stream + "." + "regexp", b"").splitlines())

    def addDecompilationTests(self, filename):
        testName = os.path.basename(filename)

        executionTest = self.addTest(megatest.ExecutionTest(
            testName,
            [self.decompiler, filename],
            stdoutFile=self.getStreamFileName(testName, "stdout"),
            stderrFile=self.getStreamFileName(testName, "stderr"),
            timeout=self.getTimeout(filename),
            expectedExitCode=self.getExpectedExitCode(filename)))

        for stream in ["stdout", "stderr"]:
            streamFile = self.getCookieFileName(filename, stream)
            if os.path.isfile(streamFile):
                self.addTest(megatest.TextFileComparisonTest(
                    "%s.%s" % (testName, stream),
                    self.getStreamFileName(testName, stream),
                    streamFile))

            for i, regexp in zip(itertools.count(1), self.getRegularExpressions(filename, stream)):
                self.addTest(megatest.RegexpFileTest(
                    "%s.%s.regexp%d" % (testName, stream, i),
                    self.getStreamFileName(testName, stream),
                    regexp))

parser = argparse.ArgumentParser(description="Runs tests on the decompiler.")
parser.add_argument("--build-dir", help="Directory where the decompiler was built.")
parser.add_argument("--scratch-dir", help="Directory for temporary files.")
parser.add_argument("--decompiler", help="Decompiler executable.")
parser.add_argument("--timeout", type=int, default=300, help="Default execution timeout, sec.")
parser.add_argument("--no-default-tests", action="store_true", default=False, help="Do not run tests from the examples directory.")
parser.add_argument("--tests-pattern", action="append", default=[], help="Glob pattern for additional test files. The option can be specified multiple times.")
args = parser.parse_args()

env = Environment(buildDirectory=args.build_dir, scratchDirectory=args.scratch_dir, decompiler=args.decompiler, timeout=args.timeout)
env.dump()

testFiles = []

if not args.no_default_tests:
    examplesDirectory = os.path.join(env.projectRoot, "examples")
    testFiles.extend(glob.glob(os.path.join(examplesDirectory, "[0-9][0-9][0-9]_*")))
    testFiles.append(os.path.join(examplesDirectory, "src"))
    testFiles.append(os.path.join(examplesDirectory, "non-existent"))

for pattern in args.tests_pattern:
    testFiles.extend(glob.glob(pattern))

testFiles.sort()

for path in testFiles:
    env.addDecompilationTests(path)

sys.exit(env.runTests())

# vim:set et sts=4 sw=4: