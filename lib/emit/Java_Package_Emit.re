open Java_Package;
open Basic_types;
open Emit_Helper;

let rec emit_package_type = (class_fn, t) => {
  let packages =
    t
    |> packages
    |> List.rev_map(package =>
         emit_package_type(class_fn, package) |> psig_module
       );
  let modules =
    // TODO: handle exception
    t |> classes |> List.rev_map(class_fn);

  let signature = List.append(packages, modules);
  module_declaration(
    ~name=Located.mk(Some(t.name |> String.capitalize_ascii)),
    ~type_=pmty_signature(signature),
  );
};
let emit_type = (env, t) =>
  emit_package_type(
    class_id => Java_Env.find(class_id, env) |> Type.emit,
    t,
  );
let emit_alias_type = t =>
  emit_package_type(
    class_id => {
      let typ_t = Java_Type.Object_Type.emit_type(class_id);
      [%sigi: type t = [%t typ_t]];
    },
    t,
  );

// TODO: classes in default package

let emit_file_type = (env, t) => {
  let packages = packages(t);
  let modules = packages |> List.map(emit_type(env));
  [%sig: open JavaFFI; [%%s [psig_recmodule(modules)]]];
};

let emit_file = (required_class, t) => [%str
  open JavaFFI;
  %s
  [Class.emit(t), Make.emit(required_class, t)]
];

let emit_file_classes = (env, t) => {
  let rec all_classes = (acc, t) => {
    let classes =
      classes(t)
      |> List.map(id => {
           // TODO: exception
           let java_class = Java_Env.find(id, env);
           let required_class =
             Java_class.find_required_classes(java_class)
             |> List.map(name =>
                  switch (Java_Env.find_opt(name, env)) {
                  | Some(clazz) => clazz
                  | None => {
                      java_name: name,
                      name,
                      extends: None,
                      fields: [],
                      constructors: [],
                      methods: [],
                      functions: [],
                    }
                  }
                );
           (id, emit_file(required_class, java_class));
         });
    let new_acc = List.append(classes, acc);
    switch (packages(t)) {
    | [] => new_acc
    | packages => packages |> List.concat_map(all_classes(new_acc))
    };
  };
  let packages = packages(t);
  packages |> List.concat_map(all_classes([]));
};
