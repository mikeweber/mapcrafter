/*
 * Copyright 2012, 2013 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "extended_ini.h"

#include "validation.h"

#include <fstream>

namespace mapcrafter {
namespace config {

ConfigSection::ConfigSection(const std::string& type, const std::string& name)
		: type(type), name(name) {
}

ConfigSection::~ConfigSection() {
}

int ConfigSection::getEntryIndex(const std::string& key) const {
	for (size_t i = 0; i < entries.size(); i++)
		if (entries[i].first == key)
			return i;
	return -1;
}

const std::string& ConfigSection::getType() const {
	return type;
}

const std::string& ConfigSection::getName() const {
	return name;
}

std::string ConfigSection::getNameType() const {
	return type + ":" + name;
}

bool ConfigSection::isNamed() const {
	return !name.empty();
}

bool ConfigSection::isEmpty() const {
	return entries.size() == 0;
}

bool ConfigSection::has(const std::string& key) const {
	return getEntryIndex(key) != -1;
}

std::string ConfigSection::get(const std::string& key, const std::string& default_value) const {
	int index = getEntryIndex(key);
	if (index == -1)
		return default_value;
	return entries[index].second;
}

const std::vector<ConfigEntry> ConfigSection::getEntries() const {
	return entries;
}

void ConfigSection::set(const std::string& key, const std::string& value) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries[index].second = value;
	else
		entries.push_back(std::make_pair(key, value));
}

void ConfigSection::remove(const std::string& key) {
	int index = getEntryIndex(key);
	if (index != -1)
		entries.erase(entries.begin() + index);
}

std::ostream& operator<<(std::ostream& out, const ConfigSection& section) {
	if (section.getName() != "") {
		if (section.getType() == "")
			out << "[" << section.getName() << "]" << std::endl;
		else
			out << "[" << section.getType() << ":" << section.getName() << "]" << std::endl;
	}

	const std::vector<ConfigEntry>& entries = section.getEntries();
	for (std::vector<ConfigEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
		out << it->first << " = " << it->second << std::endl;
	return out;
}

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
}

int ConfigFile::getSectionIndex(const std::string& type, const std::string& name) const {
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].getType() == type && sections[i].getName() == name)
			return i;
	return -1;
}

bool ConfigFile::load(std::istream& in, ValidationMessage& msg) {
	int section = -1;
	std::string line;
	int linenumber = 0;
	while (std::getline(in, line)) {
		linenumber++;
		if (line.empty())
			continue;

		// trim the line
		util::trim(line);

		// a line starting with a # is a comment
		if (line[0] == '#')
			continue;

		// a line with a new section
		else if (line[0] == '[') {
			if (line[line.size() - 1] != ']') {
				msg = ValidationMessage::error("Expecting ']' at end of line " + util::str(linenumber) + ".");
				return false;
			}

			std::string type, name;
			std::string section_name = line.substr(1, line.size() - 2);
			std::string::size_type colon = section_name.find(':');
			if (colon == std::string::npos)
				name = section_name;
			else {
				type = section_name.substr(0, colon);
				name = section_name.substr(colon+1, std::string::npos);
			}

			if (name.empty()) {
				msg = ValidationMessage::error("Invalid section name on line " + util::str(linenumber) + ".");
				return false;
			}

			section++;
			sections.push_back(ConfigSection(type, name));
		} else {
			// just a line with key = value
			std::string key, value;
			for (size_t i = 0; i < line.size(); i++) {
				if (line[i] == '=') {
					key = line.substr(0, i);
					value = line.substr(i + 1, line.size() - i - 1);
					break;
				}
				if (i == line.size() - 1) {
					msg = ValidationMessage::error("No '=' found on line " + util::str(linenumber) + ".");
					return false;
				}
			}

			util::trim(key);
			util::trim(value);

			if (section == -1)
				root.set(key, value);
			else
				sections[section].set(key, value);
		}
	}

	return true;
}

bool ConfigFile::load(std::istream& in) {
	ValidationMessage msg;
	return load(in, msg);
}

bool ConfigFile::loadFile(const std::string& filename, ValidationMessage& msg) {
	std::ifstream in(filename);
	if (!in) {
		msg = ValidationMessage::error("Unable to read file '" + filename + "'!");
		return false;
	}
	return load(in, msg);
}

bool ConfigFile::loadFile(const std::string& filename) {
	ValidationMessage msg;
	return loadFile(filename, msg);
}

bool ConfigFile::write(std::ostream& out) const {
	if (!root.isEmpty())
		out << root << std::endl;
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].isNamed())
			out << sections[i] << std::endl;
	return true;
}

bool ConfigFile::writeFile(const std::string& filename) const {
	std::ofstream out(filename);
	if (!out)
		return false;
	return write(out);
}

bool ConfigFile::hasSection(const std::string& type, const std::string& name) const {
	return getSectionIndex(type, name) != -1;
}

const ConfigSection& ConfigFile::getRootSection() const {
	return root;
}

ConfigSection& ConfigFile::getRootSection() {
	return root;
}

const std::vector<ConfigSection> ConfigFile::getSections() const {
	return sections;
}

ConfigSection& ConfigFile::addSection(const std::string& type,
		const std::string& name) {
	return getSection(type, name);
}

const ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) const {
	int index = getSectionIndex(type, name);
	if (index == -1)
		return empty_section;
	return sections.at(index);
}

ConfigSection& ConfigFile::getSection(const std::string& type, const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index != -1)
		return sections[index];
	ConfigSection section(type, name);
	sections.push_back(section);
	return sections.back();
}

ConfigSection& ConfigFile::addSection(const ConfigSection& section) {
	int index = getSectionIndex(section.getType(), section.getName());
	if (index == -1) {
		sections.push_back(section);
		return sections.back();
	}
	sections[index] = section;
	return sections[index];
}

void ConfigFile::removeSection(const std::string& type, const std::string& name) {
	int index = getSectionIndex(type, name);
	if (index == -1)
		sections.erase(sections.begin() + index);
}

} /* namespace config */
} /* namespace mapcrafter */
