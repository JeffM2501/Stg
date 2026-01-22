using System;
using System.Collections.Generic;
using System.IO;

#nullable enable

namespace codegen
{
    class Program
    {
        static void ProcessSchemaFile(string? file, string outputDir, string outputType)
        {
            if (file == null)
                return;

            List<ClassInfo> classes = new List<ClassInfo>();

            string outputFileName = Path.GetFileNameWithoutExtension(file);

            var reader = File.OpenText(file);

            string NamespaceString = string.Empty;

            ClassInfo? currentClass = null;

            var text = reader.ReadToEnd();
            text = text.Replace("\r", string.Empty);

            foreach (var line in text.Split(new char[] { '\n' }))
            {
                if (string.IsNullOrEmpty(line))
                    continue;

                var newLine = line.Trim();

                if (string.IsNullOrEmpty(newLine))
                    continue;

                var parts = newLine.Split(new char[] { ' ', '\t' }, 2);

                if (parts.Length < 1)
                    continue;

                var keyword = parts[0].ToLower();
                if (keyword == "namespace")
                {
                    NamespaceString = parts[1];
                }
                else
                {
                    if (currentClass == null)
                    {
                        currentClass = new ClassInfo();

                        currentClass.ClassType = parts[0];
                        currentClass.Name = parts[1];
                    }
                    else
                    {
                        if (keyword == "}")
                        {
                            classes.Add(currentClass);
                            currentClass = null;
                        }
                        else if (keyword != "{")
                        {
                            if (keyword.StartsWith("["))
                            {
                                currentClass.Metadata.Add(keyword, parts[1]);
                            }
                            else
                            {
                                FieldInfo field = new FieldInfo();
                                field.FieldTypename = parts[0];
                                string name = parts[1].TrimEnd(';');
                                if (name.Contains("="))
                                {
                                    var nameParts = name.Split(new char[] { '=' });
                                    name = nameParts[0].Trim();
                                    field.DefaultValue = nameParts[1].Trim();
                                }

                                field.Name = name;

                                currentClass.Fields.Add(field);
                            }
                        }
                    }
                }
            }

            string outPath = Path.Combine(outputDir, outputFileName + ".h");

            switch (outputType)
            {
                case "c++":
                    CPPGenerator.Generate(classes, NamespaceString, outPath);
                    break;

                default:
                    Console.WriteLine("Output mode " + outputDir + " is unknown");
                    break;
            }
        }

        static void Main(string[] args)
        {

            if (args.Length < 3)
            {
                Console.WriteLine("Missing args");
                return;
            }

            string OutputType = args[0];
            string SchemaDir = args[1];
            string OutputDir = args[2];

            if (string.IsNullOrEmpty(OutputDir) || !Directory.Exists(OutputDir))
            {
                Console.WriteLine("Output Dir " + OutputDir + " does not exit");
                return;
            }


            if (File.Exists(SchemaDir))
            {
                ProcessSchemaFile(SchemaDir, OutputDir, OutputType);
            }
            else if (Directory.Exists(SchemaDir)) 
            {
                foreach (var file in Directory.GetFiles(SchemaDir, "*.schema"))
                {
                    ProcessSchemaFile(file, OutputDir, OutputType);
                }
            }
            else
            {
                Console.WriteLine("Input " + SchemaDir + " is not a valid directory or file");
            }
        }
    }
}