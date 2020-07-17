open Migrate_parsetree;
open Ast_410;
open Ast_helper;
open Emit_Helper;
open Java_Type;

let (let.some) = Option.bind;

type id = Object_Type.t;

type t = {
  id,
  extends: option(id),
  // TODO: fields
  methods: list(Java_Method.t),
};

// TODO: class name to snake case
// TODO: escape override method name

let emit = t => {
  let clazz_id = "jni_jclazz";
  let object_id = "jni_jobj";
  let declare_methods = List.concat_map(Java_Method.emit, t.methods);

  let method_fields =
    t.methods
    |> List.filter(t => !t.Java_Method.static)
    |> List.map(({Java_Method.name, _}) =>
         pcf_method((
           Located.mk(name),
           Public,
           Cf.concrete(Fresh, eapply(evar(name), [evar(object_id)])),
         ))
       );
  let inheritance_field = {
    let.some extends_id = t.extends;
    let extends_lid = Object_Type.emit_lid(extends_id) |> Located.mk;
    let extends = pcl_constr(extends_lid, []);
    let apply_class = pcl_apply(extends, [(Nolabel, evar(object_id))]);
    Some(pcf_inherit(Fresh, apply_class, None));
  };
  let fields =
    List.concat([
      List.filter_map(Fun.id, [inheritance_field]),
      method_fields,
    ]);
  let class_expr = class_structure(~self=ppat_any, ~fields) |> pcl_structure;
  let class_fun = pcl_fun(Nolabel, None, pvar(object_id), class_expr);
  let class_declaration =
    class_infos(
      ~virt=Concrete,
      ~params=[],
      ~name=Located.mk(t.id.name |> String.uncapitalize_ascii),
      ~expr=class_fun // TODO: wait, class_infos without class_expr?
    );
  let find_class = {
    let name = Object_Type.to_jvm_name(t.id) |> estring;
    [%str let [%p pvar(clazz_id)] = Jni.find_class([%e name])];
  };
  List.concat([
    find_class,
    declare_methods,
    [pstr_class([class_declaration])],
  ]);
};