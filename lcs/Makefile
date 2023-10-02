C_OPT = -std=c++11 

KERNELS = lcs-classic lcs-hirschberg-opt-uneq lcs-our-alg-opt-uneq
SUITE = $(KERNELS)

.PHONY: lcs
lcs: $(SUITE)

% : %.c
	$(CXX) $(C_OPT) $< -o $@
clean:
	rm -f $(SUITE)
