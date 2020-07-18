open Java_Type;

module StringMap = Map.Make(String);

type t = {
  name: string,
  packages: StringMap.t(t),
  classes: StringMap.t(Object_Type.t),
};

let make = (~packages=StringMap.empty, ~classes=StringMap.empty, name) => {
  name,
  packages,
  classes,
};

// TODO: class and package with same name
// com.github.eduardorfs.Something
// com.github.Eduardorfs

let add_to_package = ({Object_Type.package, _} as object_type, t) => {
  let rec add_internal = (t, parts) =>
    switch (parts) {
    | [] =>
      let classes = t.classes |> StringMap.add(object_type.name, object_type);
      {...t, classes};
    | [part, ...remaining] =>
      let package = {
        t.packages
        |> StringMap.find_opt(part)
        |> Option.value(~default=make(part));
      };
      let package = add_internal(package, remaining);
      let packages = t.packages |> StringMap.add(package.name, package);
      {...t, packages};
    };
  add_internal(t, String.split_on_char('.', package));
};
