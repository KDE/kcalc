#!/usr/bin/env python3

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2016 Microsoft Corporation. All rights reserved.
# SPDX-FileCopyrightText: 2024 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>

import unittest
from appium import webdriver
from appium.webdriver.common.appiumby import AppiumBy
from appium.options.common.base import AppiumOptions
import selenium.common.exceptions
from selenium.webdriver.support.ui import WebDriverWait


class ChainResultTests(unittest.TestCase):

    @classmethod
    def setUpClass(self):
        options = AppiumOptions()
        # The app capability may be a command line or a desktop file id.
        options.set_capability("app", "org.kde.kcalc.desktop")
        # Boilerplate, always the same
        self.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)
        # Set a timeout for waiting to find elements. If elements cannot be found
        # in time we'll get a test failure. This should be somewhat long so as to
        # not fall over when the system is under load, but also not too long that
        # the test takes forever.
        self.driver.implicitly_wait = 10

    @classmethod
    def tearDownClass(self):
        # Make sure to terminate the driver again, lest it dangles.
        self.driver.quit()

    def setUp(self):
        self.driver.find_element(by=AppiumBy.NAME, value="All clear").click()

    def getresults(self):
        return self.driver.find_element(by="description", value='Result Display').text

    def assertResult(self, actual, expected):
        wait = WebDriverWait(self.driver, 20)
        try:
            wait.until(lambda x: self.getresults() == expected)
        except selenium.common.exceptions.TimeoutException:
            pass
        self.assertEqual(self.getresults(), expected)

    def test_initialize(self):
        self.driver.find_element(by=AppiumBy.NAME, value="All clear").click()

    def test_chain_binary_operator(self):
        self.driver.find_element(by=AppiumBy.NAME, value="One").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Add").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Seven").click()
        self.assertResult(self.getresults(), "8")
        self.driver.find_element(by=AppiumBy.NAME, value="Equals").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Multiply").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Two").click()
        self.assertResult(self.getresults(), "16")

    def test_chain_function(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Eight").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Nine").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Add").click()
        self.driver.find_element(by=AppiumBy.NAME, value="One").click()
        self.assertResult(self.getresults(), "90")
        self.driver.find_element(by=AppiumBy.NAME, value="Equals").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Sine").click()
        self.assertResult(self.getresults(), "1")

if __name__ == '__main__':
    unittest.main()
