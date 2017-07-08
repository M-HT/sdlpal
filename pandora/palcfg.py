#!/usr/bin/env python
# coding: UTF-8

#
#  Copyright (C) 2017 Roman Pauer
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy of
#  this software and associated documentation files (the "Software"), to deal in
#  the Software without restriction, including without limitation the rights to
#  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
#  of the Software, and to permit persons to whom the Software is furnished to do
#  so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#

import os
import sys

import pygtk
pygtk.require('2.0')
import gtk
import gobject

labelnames = [
        "title",    "language",     "display",      "audio",    "logging",
        "gamepath", "msgfile",      "fontfile",     "logfile",  "loglevel",
        "touch",    "aspect",       "fullscreen",   "cd",       "bgm",
        "opl",      "samplerate",   "stereo",       "oplrate",  "surround",
        "musvol",   "sndvol",       "buffer",       "quality",  "exit",
        "launch",   "default",      "levels"
    ]
labels = [[
        "SDLPAL Launcher",     "Language & Font",    "Display",      "Audio",      "Logging",
        "Game resource path:", "Message file:",      "Font file:",   "Log file:",  "Log level:",
        "Use touc_h overlay",  "_Keep aspect ratio", "_Full screen", "CD type:",  "BGM type:",
        "OPL type:",          "Sample rate:",       "Ste_reo",      "OPL rate:",  "Surround O_PL",
        "Music volume:",       "Sound volume:",      "Buffer:",      "Quality:",   "E_xit",
        "_Launch game",        "_Default",           "Verbose|Debug|Informational|Warning|Error|Fatal"
    ], [
        "SDLPAL 启动器",        "字体及语言设置",      "显示设置",      "音频设置",     "日志记录设置",
        "游戏资源目录：",        "语言文件：",         "字体文件",      "日志文件",     "日志记录级别：",
        "启用触屏辅助(_H)",      "保持纵横比(_K)",     "全屏模式(_F)",  "CD 音源：",   "BGM 音源：",
        "OPL 类型：",          "采样率：",           "立体声(_R)",    "OPL 采样率：", "环绕声 O_PL",
        "音乐音量：",            "音效音量：",         "缓冲区：",      "质量：",       "退出(_X)",
        "启动游戏(_L)",          "默认设置(_D)",       "详细|调试|信息|警告|错误|致命"
    ], [
        "SDLPAL 啟動器",        "字體及語言設定",      "顯示設定",      "音訊設定",      "日誌記錄設定",
        "遊戲資源檔夾：",        "語言檔：",           "字體檔：",      "日誌檔：",      "日誌記錄級別：",
        "啟用觸屏輔助(_H)",      "保持縱橫比(_K)",     "全屏模式(_F)",  "CD 音源：",    "BGM 音源：",
        "OPL 類型：",          "取樣速率：",          "立體聲(_R)",   "OPL 取樣速率：", "環繞聲 O_PL",
        "音樂音量：",            "音效音量：",         "緩衝區：",      "品質：",        "退出(_X)",
        "啟動遊戲(_L)",          "默認設定(_D)",       "詳細|調試|信息|警告|錯誤|致命"
    ]]

return_value = 0

class ConfigEntry:
    def __init__(self, entry_name, data_format, default_value):
        self.EntryName = entry_name
        self.DataFormat = data_format
        self.DefaultValue = default_value
        self.FileValue = default_value
        self.CurrentValue = default_value

        self.DataType = "Unknown"
        if "/" in self.DataFormat:
            self.DataType = "ListOfValues"
            self.ListOfValuesLow = self.DataFormat.lower().split("/")
        elif "-" in self.DataFormat:
            self.DataType = "IntegerRange"
            if (self.DataFormat.startswith("-")):
                partition = self.DataFormat[1:].partition("-")
                self.MinIntValue = - long(partition[0])
                self.MaxIntValue = long(partition[2])
            else:
                partition = self.DataFormat.partition("-")
                self.MinIntValue = long(partition[0])
                self.MaxIntValue = long(partition[2])
        elif self.DataFormat == "*":
            self.DataType = "*"
        else:
            raise BaseException, "ConfigEntry: unknown data format"


    def ResetValue(self):
        self.FileValue = self.DefaultValue
        self.CurrentValue = self.DefaultValue

    def GetFormat(self):
        return self.DataFormat

    def GetName(self):
        return self.EntryName

    def GetValue(self):
        return self.CurrentValue

    def GetValueAsIndex(self):
        if self.DataType == "ListOfValues":
            return self.ListOfValuesLow.index(self.CurrentValue.lower())
        elif self.DataType == "IntegerRange":
            return long(self.CurrentValue) - self.MinIntValue
        else:
            raise BaseException, "ConfigEntry: wrong data format"

    def IsValueChanged(self):
        return self.CurrentValue != self.FileValue

    def CorrectValue(self, value):
        if self.DataType == "ListOfValues":
            if value.lower() in self.ListOfValuesLow:
                return value
        elif self.DataType == "IntegerRange":
            try:
                ivalue = long(value)
            except ValueError:
                return self.DefaultValue

            if ivalue < self.MinIntValue:
                return str(self.MinIntValue)
            elif ivalue > self.MaxIntValue:
                return str(self.MaxIntValue)
            else:
                return value
        else:
            return value

        return self.DefaultValue

    def LoadFileValue(self, file_value):
        self.FileValue = file_value

        self.CurrentValue = self.CorrectValue(file_value)

    def RestoreDefaultValue(self):
        self.CurrentValue = self.DefaultValue

    def RestoreFileValue(self):
        self.CurrentValue = self.FileValue

    def SaveFileValue(self):
        self.FileValue = self.CurrentValue

        return self.CurrentValue

    def SetIndexAsValue(self, new_index):
        if self.DataType == "ListOfValues":
            if new_index >= 0 and new_index < len(self.ListOfValuesLow):
                self.CurrentValue = self.DataFormat.split("/")[new_index]
                return
        elif self.DataType == "IntegerRange":
            if new_index >= 0 and new_index <= self.MaxIntValue - self.MinIntValue:
                self.CurrentValue = str(new_index + self.MinIntValue)
                return
        else:
            raise BaseException, "ConfigEntry: wrong data format"

        raise BaseException, "ConfigEntry: wrong index value"

    def SetValue(self, new_value):
        if new_value == self.CorrectValue(new_value):
            self.CurrentValue = new_value
        else:
            raise BaseException, "ConfigEntry: wrong value"


class ConfigFile:
    def __init__(self, platform):
        self.Entries = dict()
        self.Order = []
        self.EOL = ""

        # misc entries
        self.AddEntry("KeepAspectRatio", "1/0", ("0" if platform == "pandora" else "1"))
        self.AddEntry("FullScreen", "1/0", ("1" if platform == "pandora" else "0"))
        self.AddEntry("LaunchSetting", "1/0", "1")
        self.AddEntry("Stereo", "1/0", "1")
        self.AddEntry("UseSurroundOPL", "1/0", ("0" if platform == "pandora" else "1"))
        self.AddEntry("UseTouchOverlay", "1/0", "0")

        self.AddEntry("SurroundOPLOffset", "-2147483648-2147483647", "384")
        self.AddEntry("LogLevel", "0-5", "5")

        self.AddEntry("AudioBufferSize", "2-32768", "1024")
        self.AddEntry("OPLSampleRate", "0-4294967295", "49716")
        self.AddEntry("ResampleQuality", "0-4", ("2" if platform == "pandora" else "4"))
        self.AddEntry("SampleRate", "0-48000", "44100")
        self.AddEntry("MusicVolume", "0-100", "100")
        self.AddEntry("SoundVolume", "0-100", "100")
        self.AddEntry("WindowHeight", "0-4294967295", ("800" if platform == "pandora" else "640"))
        self.AddEntry("WindowWidth", "0-4294967295", ("480" if platform == "pandora" else "400"))

        self.AddEntry("CD", "OGG/MP3", "OGG")
        self.AddEntry("Music", "MIDI/SOFTMIDI/RIX/MP3/OGG", "RIX")
        self.AddEntry("OPL", "DOSBOX/MAME/DOSBOXNEW", "DOSBOX")

        self.AddEntry("GamePath", "*", "")
        self.AddEntry("SavePath", "*", "")
        self.AddEntry("MessageFileName", "*", "")
        self.AddEntry("FontFileName", "*", "")
        self.AddEntry("LogFileName", "*", "")
        self.AddEntry("CLIMIDIPlayer", "*", "")


    def AddEntry(self, entry_name, data_format, default_value):
        self.Order.append(entry_name.lower())
        self.Entries[entry_name.lower()] = ConfigEntry(entry_name, data_format, default_value)

    def AreValuesChanged(self):
        for entry_lname in self.Entries.iterkeys():
            if self.Entries[entry_lname].IsValueChanged():
                return True

        return False

    def ResetValues(self):
        for entry_lname in self.Entries.iterkeys():
            self.Entries[entry_lname].ResetValue()

    def HasEntry(self, entry_name):
        return entry_name.lower() in self.Entries

    def GetEntryFormat(self, entry_name):
        return self.Entries[entry_name.lower()].GetFormat()

    def GetEntryName(self, entry_name):
        return self.Entries[entry_name.lower()].GetName()

    def GetEntryValue(self, entry_name):
        return self.Entries[entry_name.lower()].GetValue()

    def GetEntryValueAsIndex(self, entry_name):
        return self.Entries[entry_name.lower()].GetValueAsIndex()

    def SetEntryValue(self, entry_name, new_value):
        self.Entries[entry_name.lower()].SetValue(new_value)

    def SetEntryIndexAsValue(self, entry_name, new_index):
        self.Entries[entry_name.lower()].SetIndexAsValue(new_index)

    def RestoreEntryDefaultValue(self, entry_name):
        self.Entries[entry_name.lower()].RestoreDefaultValue()

    def RestoreEntryFileValue(self, entry_name):
        self.Entries[entry_name.lower()].RestoreFileValue()

    def ReadConfigFile(self, file_path):
        self.ResetValues()
        try:
            fCfg = open(file_path, "rU")
        except IOError:
            return

        for OrigLine in fCfg:
            Line = OrigLine.strip(" \r\n")
            IsEntry = False

            EqIndex = Line.find("=")

            if (not Line.startswith("#")) and (EqIndex != -1):
                EntryName = Line[:EqIndex].strip(" ")
                EntryValue = Line[EqIndex + 1:].strip(" ")

                if self.HasEntry(EntryName):
                    IsEntry = True

            if IsEntry:
                self.Entries[EntryName.lower()].LoadFileValue(EntryValue)

        if fCfg.newlines is None:
            self.EOL = ""
        elif type(fCfg.newlines) is tuple:
            self.EOL = ""
        else:
            self.EOL = fCfg.newlines

        fCfg.close()

    def WriteConfigFile(self, file_path):
        if (self.EOL == ""):
            fCfg = open(file_path, "wt")
            EOL = "\n"
        else:
            fCfg = open(file_path, "wb")
            EOL = self.EOL

        for EntryNameLow in self.Order:
            Value = self.Entries[EntryNameLow].SaveFileValue()
            if (Value != ""):
                fCfg.write(self.Entries[EntryNameLow].GetName() + "=" + Value + EOL)

        fCfg.close()


class ConfigGUI:
    def delete(self, widget, event=None):
        gtk.main_quit()
        return False

    def __init__(self, platform, file_path):
        self.file_path = file_path
        self.widgets = []
        self.labels = self.GetLabels()

        self.CfgFile = ConfigFile(platform)
        self.CfgFile.ReadConfigFile(file_path)

        if (self.CfgFile.GetEntryValue("LaunchSetting") == "0"):
            global return_value
            return_value = 11
            return

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete)

        self.window.set_size_request(790, 420 if platform == "pandora" else 460)
        self.window.set_title(self.labels.title)

        mainvbox = gtk.VBox(False, 0)
        self.window.add(mainvbox)


        # Game resource path
        hbox = gtk.HBox(False, 0)
        mainvbox.pack_start(hbox, False, True, 5)
        hbox.show()

        label = gtk.Label(self.labels.gamepath)
        hbox.pack_start(label, False, False, 5)
        label.show()

        entry = self.CreateEntry("GamePath")
        hbox.pack_end(entry, True, True)
        entry.show()


        # Language & font
        label = gtk.Label(self.labels.language)
        mainvbox.pack_start(label, False, False, 0)
        label.show()

        hbox = gtk.HBox(False, 0)
        mainvbox.pack_start(hbox, False, False, 5)
        hbox.show()

        frame = gtk.Frame()
        hbox.pack_start(frame, True, True, 5)
        frame.show()

        table = gtk.Table(2, 2, False)
        frame.add(table)
        table.show()

        # Message file
        label = gtk.Label(self.labels.msgfile)
        label.set_alignment(1, 0.5)
        table.attach(label, 0, 1, 0, 1, gtk.FILL, 0, 5, 2)
        label.show()

        entry = self.CreateEntry("MessageFileName")
        table.attach(entry, 1, 2, 0, 1, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        entry.show()

        # Font file
        label = gtk.Label(self.labels.fontfile)
        label.set_alignment(1, 0.5)
        table.attach(label, 0, 1, 1, 2, gtk.FILL, 0, 5, 2)
        label.show()

        entry = self.CreateEntry("FontFileName")
        table.attach(entry, 1, 2, 1, 2, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        entry.show()


        # Logging
        label = gtk.Label(self.labels.logging)
        mainvbox.pack_start(label, False, False, 0)
        label.show()

        hbox = gtk.HBox(False, 0)
        mainvbox.pack_start(hbox, False, False, 5)
        hbox.show()

        frame = gtk.Frame()
        hbox.pack_start(frame, True, True, 5)
        frame.show()

        table = gtk.Table(4, 1, False)
        frame.add(table)
        table.show()

        # Log Level
        label = gtk.Label(self.labels.loglevel)
        table.attach(label, 0, 1, 0, 1, 0, 0, 5, 2)
        label.show()

        combobox = self.CreateComboBox("LogLevel", self.labels.levels)
        table.attach(combobox, 1, 2, 0, 1, 0, 0, 5, 2)
        combobox.show()

        # Log file
        label = gtk.Label(self.labels.logfile)
        table.attach(label, 2, 3, 0, 1, 0, 0, 5, 2)
        label.show()

        entry = self.CreateEntry("LogFileName")
        table.attach(entry, 3, 4, 0, 1, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        entry.show()


        if platform != "pandora":
            # Display
            label = gtk.Label(self.labels.display)
            mainvbox.pack_start(label, False, False, 0)
            label.show()

            hbox = gtk.HBox(False, 0)
            mainvbox.pack_start(hbox, False, False, 5)
            hbox.show()

            frame = gtk.Frame()
            hbox.pack_start(frame, True, True, 5)
            frame.show()

            table = gtk.Table(7, 1, False)
            frame.add(table)
            table.show()

            # Use touch overlay
            checkbutton = self.CreateCheckButton(self.labels.touch, "UseTouchOverlay")
            table.attach(checkbutton, 1, 2, 0, 1, gtk.EXPAND, 0, 5, 2)
            checkbutton.show()

            # Keep aspect ratio
            checkbutton = self.CreateCheckButton(self.labels.aspect, "KeepAspectRatio")
            table.attach(checkbutton, 3, 4, 0, 1, gtk.EXPAND, 0, 5, 2)
            checkbutton.show()

            # Full screen
            checkbutton = self.CreateCheckButton(self.labels.fullscreen, "FullScreen")
            table.attach(checkbutton, 5, 6, 0, 1, gtk.EXPAND, 0, 5, 2)
            checkbutton.show()


        # Audio
        label = gtk.Label(self.labels.audio)
        mainvbox.pack_start(label, False, False, 0)
        label.show()

        hbox = gtk.HBox(False, 0)
        mainvbox.pack_start(hbox, False, False, 5)
        hbox.show()

        frame = gtk.Frame()
        hbox.pack_start(frame, True, True, 5)
        frame.show()

        vbox = gtk.VBox(False, 0)
        frame.add(vbox)
        vbox.show()

        table = gtk.Table(8, 2, False)
        vbox.pack_start(table, False, False, 0)
        table.show()

        # CD type
        label = gtk.Label(self.labels.cd)
        label.set_alignment(1, 0.5)
        table.attach(label, 0, 1, 0, 1, gtk.FILL, 0, 5, 2)
        label.show()

        combobox = self.CreateComboBox("CD")
        table.attach(combobox, 1, 2, 0, 1, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        combobox.show()

        # BGM type
        label = gtk.Label(self.labels.bgm)
        label.set_alignment(1, 0.5)
        table.attach(label, 2, 3, 0, 1, gtk.FILL, 0, 5, 2)
        label.show()

        combobox = self.CreateComboBox("Music")
        table.attach(combobox, 3, 4, 0, 1, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        combobox.show()

        # Stereo
        checkbutton = self.CreateCheckButton(self.labels.stereo, "Stereo")
        table.attach(checkbutton, 4, 5, 0, 1, gtk.FILL, 0, 5, 2)
        checkbutton.show()

        # Sample rate
        label = gtk.Label(self.labels.samplerate)
        label.set_alignment(1, 0.5)
        table.attach(label, 5, 7, 0, 1, gtk.FILL, 0, 5, 2)
        label.show()

        entry = self.CreateEntry("SampleRate")
        table.attach(entry, 7, 8, 0, 1, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        entry.show()

        # OPL type
        label = gtk.Label(self.labels.opl)
        label.set_alignment(1, 0.5)
        table.attach(label, 0, 1, 1, 2, gtk.FILL, 0, 5, 2)
        label.show()

        combobox = self.CreateComboBox("OPL")
        table.attach(combobox, 1, 2, 1, 2, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        combobox.show()

        # OPL rate
        label = gtk.Label(self.labels.oplrate)
        label.set_alignment(1, 0.5)
        table.attach(label, 2, 3, 1, 2, gtk.FILL, 0, 5, 2)
        label.show()

        entry = self.CreateEntry("OPLSampleRate")
        table.attach(entry, 3, 4, 1, 2, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        entry.show()

        # Surround OPL
        checkbutton = self.CreateCheckButton(self.labels.surround, "UseSurroundOPL")
        table.attach(checkbutton, 4, 6, 1, 2, gtk.FILL, 0, 5, 2)
        checkbutton.show()

        # Buffer
        label = gtk.Label(self.labels.buffer)
        label.set_alignment(1, 0.5)
        table.attach(label, 6, 7, 1, 2, gtk.FILL, 0, 5, 2)
        label.show()

        entry = self.CreateEntry("AudioBufferSize")
        table.attach(entry, 7, 8, 1, 2, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        entry.show()

        # Quality
        table = gtk.Table(4, 2, False)
        vbox.pack_start(table, False, False, 0)
        table.show()

        label = gtk.Label(self.labels.quality)
        label.set_alignment(1, 0.5)
        table.attach(label, 0, 1, 0, 1, gtk.FILL, 0, 5, 2)
        label.show()

        hscale = self.CreateScale("ResampleQuality")
        table.attach(hscale, 1, 2, 0, 1, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        hscale.show()

        # Music volume
        label = gtk.Label(self.labels.musvol)
        label.set_alignment(1, 0.5)
        table.attach(label, 2, 3, 0, 1, gtk.FILL, 0, 5, 2)
        label.show()

        hscale = self.CreateScale("MusicVolume")
        table.attach(hscale, 3, 4, 0, 1, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        hscale.show()

        # Sound volume
        label = gtk.Label(self.labels.sndvol)
        label.set_alignment(1, 0.5)
        table.attach(label, 2, 3, 1, 2, gtk.FILL, 0, 5, 2)
        label.show()

        hscale = self.CreateScale("SoundVolume")
        table.attach(hscale, 3, 4, 1, 2, gtk.EXPAND | gtk.FILL, 0, 5, 2)
        hscale.show()


        # Create a bunch of buttons
        hbox = gtk.HBox(True, 0)
        mainvbox.pack_end(hbox, False, False, 5)
        hbox.show()

        button = gtk.Button(self.labels.exit)
        button.connect("clicked", self.Exit)
        hbox.pack_start(button, False, True, 0)
        button.show()

        button = gtk.Button(self.labels.launch)
        button.connect("clicked", self.Launch)
        hbox.pack_start(button, False, True, 0)
        button.show()

        button = gtk.Button(self.labels.default)
        button.connect("clicked", self.LoadDefaultValues)
        hbox.pack_start(button, False, True, 0)
        button.show()

        mainvbox.show()
        self.window.show()


    def GetLanguage(self):
        lang = os.getenv("LANG")
        if lang is None:
            return 0

        if lang.lower().startswith("zh_"):
            lang2 = lang[3:].lower()

            if (lang2.startswith("hans") or lang2.startswith("cn") or lang2.startswith("sg") or lang2.startswith("chs")):
                return 1
            else:
                return 2
        else:
            return 0

    def GetLabels(self):
        global labelnames
        global labels

        lang = self.GetLanguage()
        langlabels = lambda: None
        for i in range(len(labelnames)):
            setattr(langlabels, labelnames[i], labels[lang][i])

        return langlabels

    def Exit(self, widget, data=None):
        self.delete(self.window)

    def Launch(self, widget, data=None):
        global return_value
        return_value = 11
        self.CfgFile.SetEntryValue("LaunchSetting", "0")
        self.CfgFile.WriteConfigFile(self.file_path)
        self.delete(self.window)

    def EntryChanged(self, widget, data=None):
        self.CfgFile.SetEntryValue(data, widget.get_text())

    def ComboBoxChanged(self, widget, data=None):
        self.CfgFile.SetEntryIndexAsValue(data, widget.get_active())

    def CheckButtonChanged(self, widget, data=None):
        self.CfgFile.SetEntryValue(data, "1" if widget.get_active() else "0")

    def ScaleChanged(self, widget, data=None):
        self.CfgFile.SetEntryValue(data, str(long(widget.get_value())))


    def LoadDefaultValues(self, widget, data=None):
        for entry_type, entry_name, widget in self.widgets:
            self.CfgFile.RestoreEntryDefaultValue(entry_name)

        self.DisplayValues()


    def DisplayValues(self):
        for entry_type, entry_name, widget in self.widgets:
            if entry_type == "combobox":
                widget.set_active(self.CfgFile.GetEntryValueAsIndex(entry_name))
            else:
                current_value = self.CfgFile.GetEntryValue(entry_name)
                if entry_type == "entry":
                    widget.set_text(current_value)
                elif entry_type == "scale":
                    widget.set_value(long(current_value))
                elif entry_type == "checkbutton":
                    widget.set_active(current_value != "0")
                elif entry_type == "radioset":
                    for button in widget:
                        button.set_active(button.get_label() == current_value)


    def CreateEntry(self, entry_name):
        maxlen = 256
        entry_format = self.CfgFile.GetEntryFormat(entry_name)
        if "-" in entry_format:
            if (entry_format.startswith("-")):
                partition = entry_format[1:].partition("-")
            else:
                partition = entry_format.partition("-")

            maxlen = max(len(partition[0]), len(partition[2]))

        entry = gtk.Entry(maxlen)
        if (maxlen < 256):
            entry.set_width_chars(1 + maxlen)
        entry.set_text(self.CfgFile.GetEntryValue(entry_name))
        entry.connect("changed", self.EntryChanged, entry_name)

        self.widgets.append(("entry", entry_name, entry))

        return entry

    def CreateComboBox(self, combobox_name, combobox_values = None):
        store = gtk.ListStore(gobject.TYPE_STRING)
        combobox = gtk.ComboBox(store)

        cell = gtk.CellRendererText();
        combobox.pack_start(cell, True)
        combobox.add_attribute(cell, 'text', 0)

        if not combobox_values is None:
            combobox_values = combobox_values.split("|")
        else:
            combobox_values = self.CfgFile.GetEntryFormat(combobox_name).split("/")

        for combobox_value in combobox_values:
            store.append([combobox_value])

        combobox.set_active(self.CfgFile.GetEntryValueAsIndex(combobox_name))
        combobox.connect("changed", self.ComboBoxChanged, combobox_name)

        self.widgets.append(("combobox", combobox_name, combobox))

        return combobox

    def CreateCheckButton(self, checkbutton_label, checkbutton_name):
        checkbutton = gtk.CheckButton(checkbutton_label)
        checkbutton.set_active(self.CfgFile.GetEntryValue(checkbutton_name) != "0")
        checkbutton.connect("toggled", self.CheckButtonChanged, checkbutton_name)

        self.widgets.append(("checkbutton", checkbutton_name, checkbutton))

        return checkbutton

    def CreateScale(self, scale_name):
        entry_format = self.CfgFile.GetEntryFormat(scale_name).partition("-")
        current_value = long(self.CfgFile.GetEntryValue(scale_name))
        adjustment = gtk.Adjustment(current_value, long(entry_format[0]), long(entry_format[2]), 1, 1)
        hscale = gtk.HScale(adjustment)
        hscale.set_digits(0)
        hscale.connect("value-changed", self.ScaleChanged, scale_name)

        self.widgets.append(("scale", scale_name, hscale))

        return hscale


if len(sys.argv) >= 3:
    ConfigGUI(sys.argv[1], sys.argv[2])
    if return_value == 0:
        gtk.main()
else:
    print "Not enough parameters: platform file_path"
    print "\tplatform = pc / pandora"

sys.exit(return_value)
