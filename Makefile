WAF=python ext/waf-sfiera/waf

all:
	@$(WAF) build

test:
	@$(WAF) test

clean:
	@$(WAF) clean

dist:
	@$(WAF) dist

distclean:
	@$(WAF) distclean

.PHONY: clean dist distclean all test
