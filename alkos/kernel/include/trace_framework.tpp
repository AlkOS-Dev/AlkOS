

namespace trace
{
template <TraceType type, TraceModule module, class... Args>
void Write(const char *format, Args... args);
}  // namespace  trace
