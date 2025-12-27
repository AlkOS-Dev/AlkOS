extern "C" void (*g_print_fn)(const char *);

void print(const char *msg)
{
    if (g_print_fn) {
        g_print_fn(msg);
    }
}

extern "C" int main()
{
    print("Hello from User Space (Address 0x400000)!\n");
    print("I was loaded from the file system.\n");
    return 0;
}
