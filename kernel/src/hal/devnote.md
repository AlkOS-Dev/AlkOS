## Purpose
Files here are wrappers that *expect* arch code to satisfy the API, include the relevant arch code and transfer it from the arch namespace, to the hardware abstraction layer (hal) namespace.

No functions from arch should be used in kernel outside of the HAL.
