#pragma once
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdint.h>

namespace darkroom
{
	// "program.exe 1.txt 2.txt --arg 1.23f --skip file.txt --inplace"
	// in the example above:
	// "--arg 1.23f" and "--skip file.txt" are regular arguments
	// "--inplace" is singular argument
	// "1.txt" and "2.txt" are free arguments
	// the following class helps parsing these types of arguments
	class CmdLineParser
	{
	public:
		void addRegularOption(const std::string& name, const std::string& description) { m_regularOptions.push_back({ name, description });	}
		void addSingularOption(const std::string& name, const std::string& description)	{ m_singularOptions.push_back({ name, description } ); }

		bool parse(int argc, char** argv)
		{
			if (argc < 0 || argv == nullptr)
				return false;

			std::vector<std::string> args;
			for (int i = 1; i < argc; ++i)
				if (argv[i])
					args.push_back(argv[i]);
				else
					return false;

			auto processArgument = [&](const std::string& optName, size_t& i) -> bool
			{
				if (canParseOption(optName, m_regularOptions))
				{
					if (i + 1 < args.size())
					{
						m_options[optName] = args[i + 1];
						i += 1;
					}
					else
						return false;
				}
				else if (canParseOption(optName, m_singularOptions))
					m_options[optName] = std::string("");
				else
					m_freeOptions.push_back(args[i]);

				return true;
			};

			for (size_t i = 0u; i < args.size(); ++i)
			{
				// --arg, -arg or arg
				if (args[i].size() > 2 && args[i][0] == '-' && args[i][1] == '-')
				{
					std::string optName(args[i].cbegin() + 2, args[i].cend());
					if (!processArgument(optName, i))
						return false;
				}
				else if (args[i].size() > 1 && args[i][0] == '-')
				{
					std::string optName(args[i].cbegin() + 1, args[i].cend());
					if (!processArgument(optName, i))
						return false;
				}
				else
					m_freeOptions.push_back(args[i]);
			}

			return true;
		}

		bool hasOption(const std::string& opt) const { return m_options.count(opt) != 0; }
		size_t freeOptionsCount() const { return m_freeOptions.size(); }
		// return an argument specified with index as std::string, float or uint64_t
		template<typename T> T getOptionAs(const std::string& opt)
		{
			if (!hasOption(opt))
				return T();

			T result = T();
			std::stringstream ss(m_options[opt]);
			ss >> result;
			return result;
		}

		template<> std::string getOptionAs(const std::string& opt)
		{
			if (!hasOption(opt))
				return std::string();

			return m_options[opt];
		}

		// return a free floating argument specified with index as std::string, float or uint64_t
		template<typename T> T getFreeOptionAs(size_t index)
		{
			if (index < m_freeOptions.size())
			{
				T result = T();
				std::stringstream ss(m_freeOptions[index]);
				ss >> result;
				return result;
			}

			return T();
		}

		template<> std::string getFreeOptionAs(size_t index)
		{
			if (index < m_freeOptions.size())
				return m_freeOptions[index];

			return std::string();
		}


		void printUsage() const
		{
			for (auto& opt : m_regularOptions)
				std::cout << "\t--" << opt.first << "\t" << opt.second << std::endl;
			for (auto& opt : m_singularOptions)
				std::cout << "\t--" << opt.first << "\t" << opt.second << std::endl;
		}
	private:
		bool canParseOption(const std::string& name, const std::vector<std::pair<std::string, std::string>>& opts)
		{
			for (auto& opt : opts)
				if (opt.first == name)
					return true;
			return false;
		}

		std::vector<std::pair<std::string, std::string>> m_singularOptions;
		std::vector<std::pair<std::string, std::string>> m_regularOptions;

		std::vector<std::string> m_freeOptions;
		std::map<std::string, std::string> m_options;
	};
}