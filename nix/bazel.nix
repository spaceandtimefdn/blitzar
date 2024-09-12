{ pkgs, clang, cuda }:
let
  # Set PATH so that it only includes things bazel needs.
  path = pkgs.lib.strings.concatStringsSep ":" [
    "${clang}/bin"
    "${pkgs.git}/bin"
    "${pkgs.portableGcc}/bin"
    "${pkgs.portableGcc.libc.bin}/bin"
    "${pkgs.binutils}/bin"
    "${pkgs.coreutils}/bin"
    "${pkgs.findutils}/bin"
    "${pkgs.gnused}/bin"
    "${pkgs.gnugrep}/bin"
    "${pkgs.bash}/bin"
    "${cuda}/bin"
  ];
  bazel = "${pkgs.bazel_7}/bin/bazel";
in
pkgs.writeShellScriptBin "bazel" ''
  if [[
    "$1" == "build" ||
    "$1" == "test" ||
    "$1" == "run"
  ]]; then
    exec ${bazel} $1 \
     --action_env CC=${clang}/bin/clang \
     --action_env CXX=${clang}/bin/clang++ \
     --action_env PATH="${path}" \
     --action_env CUDA_PATH="${cuda}" \
     --action_env=BAZEL_LINKLIBS='-l%:libc++.a' \
     --action_env=BAZEL_LINKOPTS='-L${clang}/lib' \
     ''${@:2}
  else
    exec ${bazel} $@
  fi''

