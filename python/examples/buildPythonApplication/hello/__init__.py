import nix

def greet():
    print("Evaluating 1 + 1 in Nix gives:" + str(nix.eval("1 + 1")))
