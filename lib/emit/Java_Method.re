open Basic_types;

let find_required_classes = t =>
  List.concat_map(
    ((_, java_type)) => Java_Type.find_required_class(java_type),
    t.parameters,
  )
  @ Java_Type.find_required_class(t.return_type);

let relativize = (clazz, t) => {
  let relativize = Java_Type.relativize(clazz);
  let parameters =
    t.parameters |> List.map(((name, value)) => (name, relativize(value)));
  let return_type = relativize(t.return_type);
  {...t, parameters, return_type};
};
