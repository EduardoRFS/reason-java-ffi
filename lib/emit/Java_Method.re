// TODO: access
[@deriving show]
type t = {
  // TODO: think on a better way to keep these copies
  java_name: string,
  java_parameters: list((string, Java_Type.t)),
  java_return_type: Java_Type.t,
  name: string,
  static: bool,
  parameters: list((string, Java_Type.t)),
  return_type: Java_Type.t,
};

let (let.ok) = Result.bind;

let to_jvm_signature = t => {
  let args =
    t.parameters
    |> List.map(((_, java_type)) => Java_Type.to_jvm_signature(java_type))
    |> String.concat("");
  let ret = Java_Type.to_jvm_signature(t.return_type);
  "(" ++ args ++ ")" ++ ret;
};

let find_required_classes = t =>
  List.concat_map(
    ((_, java_type)) => Java_Type.find_required_class(java_type),
    t.java_parameters,
  )
  @ Java_Type.find_required_class(t.java_return_type);

let relativize = (clazz, t) => {
  let relativize = Java_Type.relativize(clazz);
  let parameters =
    t.parameters |> List.map(((name, value)) => (name, relativize(value)));
  let return_type = relativize(t.return_type);
  {...t, parameters, return_type};
};
