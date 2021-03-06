#include "config.h"
#include "Output.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include "Hash.h"
#include "Options.h"

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::algorithm;
using namespace arx;

const uint32 outTaskRequirement[] = {
  H_CRC,
  H_MD5,
  H_ED2K,
  H_SHA1,
  H_UNKNOWN
};

namespace detail {

  string conditionalUpperCase(string digest) {
    return (options.isUpperCase()) ? to_upper_copy(digest) : digest;
  }

  void writeHeader(string commentStart, Printer* printer) {
    ptime now = second_clock::local_time();
    *printer << commentStart << " Generated by ArXSum " << VERSION << " on " << 
      to_iso_extended_string(now.date()) << " at " << 
      to_simple_string(now.time_of_day()) << "\n";
    *printer << commentStart << " ArXSum includes cryptographic software written by Eric Young (eay@cryptsoft.com)" << "\n";
    *printer << commentStart << " (c) Alexander 'Elric' Fokin, [ArX] Team, 2007" << "\n";
  }

  void writeTimesAndSizes(string commentStart, ArrayList<FileEntry> data, Printer* printer) {
    FOREACH(FileEntry file, data) {
      if(!file.isFailed()) {
        ptime fileTime = from_time_t(file.getDateTime());
        *printer << commentStart << " " << setw(12) << file.getSize() << "  " << 
           to_simple_string(fileTime.time_of_day()) << " " << 
           to_iso_extended_string(fileTime.date()) << " " << file.getPath() << "\n";
      }
    }
  }

  class OutputFormatImpl {
  private:
  public:
    virtual void output(HashTask task, ArrayList<FileEntry> data, Printer* printer) = 0;
    virtual string getName() = 0;
  };

  class MD5OutputFormat: public OutputFormatImpl {
  private:
  public:
    void output(HashTask task, ArrayList<FileEntry> data, Printer* printer) {
      writeHeader(";", printer);
      *printer << ";" << "\n";
      writeTimesAndSizes(";", data, printer);
      for(uint32 n = 0; n < H_COUNT; n++) {
        if(task.isSet(n) && n != H_MD5) {
          *printer << ";" << "\n" << "; * " << Hash::getName(n) << " Block *" << "\n";
          FOREACH(FileEntry file, data)
            if(!file.isFailed())
              *printer << "; " << conditionalUpperCase(file.getDigest(n).toHexString()) << " *" << file.getPath() << "\n";
        }
      }
      FOREACH(FileEntry file, data)
        if(!file.isFailed())
          *printer << conditionalUpperCase(file.getDigest(H_MD5).toHexString()) << " *" << file.getPath() << "\n";
    }
    string getName() {
      return "MD5";
    }
  };

  class SHA1OutputFormat: public OutputFormatImpl {
  private:
  public:
    void output(HashTask task, ArrayList<FileEntry> data, Printer* printer) {
      writeHeader(";", printer);
      *printer << ";" << "\n";
      writeTimesAndSizes(";", data, printer);
      FOREACH(FileEntry file, data)
        if(!file.isFailed())
          *printer << conditionalUpperCase(file.getDigest(H_SHA1).toHexString()) << " *" << file.getPath() << "\n";
    }
    string getName() {
      return "SHA1";
    }
  };

  class BSDOutputFormat: public OutputFormatImpl {
  private:
  public:
    void output(HashTask task, ArrayList<FileEntry> data, Printer* printer) {
      FOREACH(FileEntry file, data) if(!file.isFailed())
        for(uint32 n = 0; n < H_COUNT; n++) if(task.isSet(n))
          *printer << Hash::getName(n) << " (" << file.getPath() << ") = " << conditionalUpperCase(file.getDigest(n).toHexString()) << "\n";
    }
    string getName() {
      return "BSD";
    }
  };

  class SFVOutputFormat: public OutputFormatImpl {
  private:
  public:
    void output(HashTask task, ArrayList<FileEntry> data, Printer* printer) {
      writeHeader(";", printer);
      *printer << ";" << "\n";
      writeTimesAndSizes(";", data, printer);
      FOREACH(FileEntry file, data)
        if(!file.isFailed())
          *printer << file.getPath() << " " << conditionalUpperCase(file.getDigest(H_CRC).toHexString()) << "\n";
    }
    string getName() {
      return "SFV";
    }
  };

  class ED2KOutputFormat: public OutputFormatImpl {
  private:
  public:
    void output(HashTask task, ArrayList<FileEntry> data, Printer* printer) {
      //ed2k://|file|Amaenaide yo!! Katsu!! - 01 =Mendoi=.avi|244576256|bd4bffffc7664e11e85485383c984507|/
      FOREACH(FileEntry file, data)
        if(!file.isFailed())
          *printer << "ed2k://|file|" << file.getPath().leaf() << "|" << 
            file.getSize() << "|" << conditionalUpperCase(file.getDigest(H_ED2K).toHexString()) << "|/" << "\n";
    }
    string getName() {
      return "ED2K";
    }
  };
};

OutputFormat::OutputFormat(uint32 ofId) {
  switch(ofId) {
  case O_MD5:
    impl.reset(new ::detail::MD5OutputFormat());
    break;
  case O_SFV:
    impl.reset(new ::detail::SFVOutputFormat());
    break;
  case O_ED2K:
    impl.reset(new ::detail::ED2KOutputFormat());
    break;
  case O_SHA1:
    impl.reset(new ::detail::SHA1OutputFormat());
    break;
  case O_BSD:
    impl.reset(new ::detail::BSDOutputFormat());
    break;
  default:
    throw new std::runtime_error("Unknown Output Format Id: " + lexical_cast<string>(ofId));
  }
}

void OutputFormat::output(HashTask task, ArrayList<FileEntry> data, Printer* printer) {
  impl->output(task, data, printer);
}

string OutputFormat::getName() {
  return impl->getName();
}

std::string OutputFormat::getName(uint32 ofId) {
  return OutputFormat(ofId).getName();
}

uint32 OutputFormat::getId() {
  return this->ofId;
}



