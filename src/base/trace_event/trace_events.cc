#ifdef LIBBASE_ENABLE_TRACING

#include "base/trace_event/trace_events.h"

namespace base {
namespace detail {

namespace {
void WriteArgumentsToStream(std::ostream& stream,
                            const ArgumentPacker::StringArguments& args) {
  stream << "\"args\":{";
  for (const auto& arg : args) {
    stream << "\"" << arg.first << "\":\"" << arg.second << "\"";
    if (&arg != &args.back()) {
      stream << ",";
    }
  }
  stream << "}";
}

void WriteArgumentsToStream(std::ostream& stream,
                            const ArgumentPacker::IntegerArguments& args) {
  stream << "\"args\":{";
  for (const auto& arg : args) {
    stream << "\"" << arg.first << "\":" << arg.second;
    if (&arg != &args.back()) {
      stream << ",";
    }
  }
  stream << "}";
}
}  // namespace

void TraceEvent::WriteTo(std::ostream& stream) const {
  stream << "{";
  {
    stream << "\"name\":\"" << name << "\",";
    stream << "\"cat\":\"" << cat << "\",";
    if (!id.empty()) {
      stream << "\"id\":\"" << id << "\",";
    }
    stream << "\"ph\":\"" << ph << "\",";
    stream << "\"ts\":" << ts << ",";
    stream << "\"pid\":" << pid << ",";
    stream << "\"tid\":" << tid << ",";
    WriteArgumentsToStream(stream, args);
  }
  stream << "}";
}

void TraceCompleteEvent::WriteTo(std::ostream& stream) const {
  stream << "{";
  {
    stream << "\"name\":\"" << name << "\",";
    stream << "\"cat\":\"" << cat << "\",";
    stream << "\"ph\":\"" << 'X' << "\",";
    stream << "\"ts\":" << ts << ",";
    stream << "\"dur\":" << dur << ",";
    stream << "\"pid\":" << pid << ",";
    stream << "\"tid\":" << tid << ",";
    WriteArgumentsToStream(stream, args);
  }
  stream << "}";
}

void TraceCounter::WriteTo(std::ostream& stream) const {
  stream << "{";
  {
    stream << "\"name\":\"" << name << "\",";
    stream << "\"cat\":\"" << cat << "\",";
    stream << "\"ph\":\"" << 'C' << "\",";
    stream << "\"ts\":" << ts << ",";
    stream << "\"pid\":" << pid << ",";
    WriteArgumentsToStream(stream, args);
  }
  stream << "}";
}

void TraceCounterId::WriteTo(std::ostream& stream) const {
  stream << "{";
  {
    stream << "\"name\":\"" << name << "\",";
    stream << "\"cat\":\"" << cat << "\",";
    stream << "\"id\":\"" << id << "\",";
    stream << "\"ph\":\"" << 'C' << "\",";
    stream << "\"ts\":" << ts << ",";
    stream << "\"pid\":" << pid << ",";
    WriteArgumentsToStream(stream, args);
  }
  stream << "}";
}

void TraceInstantEvent::WriteTo(std::ostream& stream) const {
  stream << "{";
  {
    stream << "\"name\":\"" << name << "\",";
    stream << "\"cat\":\"" << cat << "\",";
    stream << "\"s\":\"" << s << "\",";
    stream << "\"ph\":\"" << 'i' << "\",";
    stream << "\"ts\":" << ts << ",";
    stream << "\"pid\":" << pid << ",";
    stream << "\"tid\":" << tid << ",";
    WriteArgumentsToStream(stream, args);
  }
  stream << "}";
}

}  // namespace detail
}  // namespace base

#endif
