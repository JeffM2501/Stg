using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text;

namespace codegen
{
    internal static class CPPGenerator
    {
        internal static string GetCPPType(string typename)
        {
            switch(typename)
            {
                default:  return "uint32_t";

                case "float": return "float";
                case "double": return "double";
                case "u32": return "uint32_t";
                case "u64": return "uint64_t";
                case "string": return "std::string_view";
                case "bool": return "bool";
            }
        }

        internal static string GetPacketSizeString(ClassInfo info)
        {
            StringBuilder builder = new StringBuilder();

            builder.Append("AllocatePacket(");
            bool first = true;
            foreach (var field in info.Fields)
            {
                if (!first)
                {
                    builder.Append(" + ");
                }

                string lowerName = char.ToLower(field.Name[0]) + field.Name.Substring(1);

                if (field.FieldTypename == "string")
                {
                    builder.Append("GetBufferWriteSize(" + lowerName + ".size())");
                }
                else
                {
                    builder.Append("sizeof(" + GetCPPType(field.FieldTypename) + ")");
                }
                
                first = false;
            }
            builder.Append(");");
            return builder.ToString();
        }

        public static void Generate(List<ClassInfo> classes, string namespaceName, string outputFile)
        {
            bool hasMessages = false;
            foreach (var obj in classes)
            {
                if (obj.ClassType == "message")
                {
                    hasMessages = true; break;
                }
            }

            File.Delete(outputFile);
            var writer = File.OpenWrite(outputFile);
            if (!writer.CanWrite)
                return;

            StreamWriter textWriter = new StreamWriter(writer);

            textWriter.WriteLine("#pragma once");
            textWriter.WriteLine();

            if (hasMessages)
            {
                textWriter.WriteLine("#include \"messages.h\""); 
                textWriter.WriteLine("#include \"crc64.h\"");
                textWriter.WriteLine();
                textWriter.WriteLine("namespace MessageIDS");
                textWriter.WriteLine("{");
                foreach (var classInfo in classes)
                {
                    if (classInfo.ClassType != "message")
                        continue;
                    textWriter.WriteLine("\tstatic const uint64_t " + classInfo.Name + " = Hashes::CRC64Str(\"" + classInfo.Name + "\");");
                }
                textWriter.WriteLine("}");

                textWriter.WriteLine();
            }

            if (!string.IsNullOrEmpty(namespaceName))
            {
                textWriter.WriteLine("namespace " + namespaceName);
                textWriter.WriteLine("{");
            }

            foreach (var classInfo in classes)
            {
                if (classInfo.ClassType == "message")
                {
                    // class
                    textWriter.WriteLine("\tclass " + classInfo.Name + " :public MessageBuffer");
                    textWriter.WriteLine("\t{");
                    textWriter.WriteLine("\tpublic:");
                    textWriter.WriteLine("\t\tDECLARE_MESSAGE_ID(MessageIDS::" + classInfo.Name + ");");

                    // write constructor
                    textWriter.Write("\t\t" + classInfo.Name + "(");

                    bool first = true;
                    foreach (FieldInfo fieldInfo in classInfo.Fields)
                    {
                        if (string.IsNullOrEmpty(fieldInfo.DefaultValue))
                        {
                            if (!first)
                                textWriter.Write(", ");

                            string lowerName = char.ToLower(fieldInfo.Name[0]) + fieldInfo.Name.Substring(1);
                            textWriter.Write(GetCPPType(fieldInfo.FieldTypename) + " " + lowerName);
                            first = false;
                        }                        
                    }

                    foreach (FieldInfo fieldInfo in classInfo.Fields)
                    {
                        if (!string.IsNullOrEmpty(fieldInfo.DefaultValue))
                        {
                            if (!first)
                                textWriter.Write(", ");

                            string lowerName = char.ToLower(fieldInfo.Name[0]) + fieldInfo.Name.Substring(1);
                            textWriter.Write(GetCPPType(fieldInfo.FieldTypename) + " " + lowerName + " = " + fieldInfo.DefaultValue);
                            first = false;
                        }
                    }
                    textWriter.WriteLine(") : MessageBuffer(nullptr)");

                    textWriter.WriteLine("\t\t{");
                    if (classInfo.Metadata.ContainsKey("[channel]"))
                    {
                        textWriter.WriteLine("\t\t\tChannel = NetworkChannelIDs::" + classInfo.Metadata["[channel]"] + ";");
                    }
                    textWriter.WriteLine("\t\t\t" + GetPacketSizeString(classInfo));

                    // field offsets
                    textWriter.WriteLine();
                    textWriter.WriteLine("\t\t\tsize_t offset = 0;");

                    foreach (FieldInfo fieldInfo in classInfo.Fields)
                    {
                        textWriter.WriteLine("\t\t\t" + fieldInfo.Name + "Offset = offset;");

                        if (fieldInfo.FieldTypename == "string")
                        {
                            string lowerName = char.ToLower(fieldInfo.Name[0]) + fieldInfo.Name.Substring(1);
                            textWriter.WriteLine("\t\t\toffset += GetBufferWriteSize(" + lowerName + ".size());");
                        }
                        else
                        {
                            textWriter.WriteLine("\t\t\toffset += sizeof(" + GetCPPType(fieldInfo.FieldTypename) + ");");
                        }
                    }

                    // type IDs
                    textWriter.WriteLine();
                    textWriter.WriteLine("\t\t\tWriteTypeID(MessageIDS::" + classInfo.Name + ");");

                    foreach (FieldInfo fieldInfo in classInfo.Fields)
                    {
                        string lowerName = char.ToLower(fieldInfo.Name[0]) + fieldInfo.Name.Substring(1);
                        string setFunction = "Set" + fieldInfo.Name;
                        textWriter.WriteLine("\t\t\t" + setFunction + "(" + lowerName+ ");");
                    }
                    textWriter.WriteLine("\t\t}");

                    // read constructor
                    textWriter.WriteLine();
                    textWriter.WriteLine("\t\t" + classInfo.Name + "(ENetPacket* packet) : MessageBuffer(packet)");
                    textWriter.WriteLine("\t\t{");
                    if (classInfo.Metadata.ContainsKey("[channel]"))
                        textWriter.WriteLine("\t\t\tChannel = NetworkChannelIDs::" + classInfo.Metadata["[channel]"] + ";");

                    textWriter.WriteLine();
                    textWriter.WriteLine("\t\t\tsize_t offset = 0;");

                    foreach (FieldInfo fieldInfo in classInfo.Fields)
                    {
                        textWriter.WriteLine("\t\t\t" + fieldInfo.Name + "Offset = offset;");

                        if (fieldInfo.FieldTypename == "string")
                        {
                            string lowerName = char.ToLower(fieldInfo.Name[0]) + fieldInfo.Name.Substring(1);
                            textWriter.WriteLine("\t\t\toffset += ReadBufferSize(" + fieldInfo.Name + "Offset);");
                        }
                        else
                        {
                            textWriter.WriteLine("\t\t\toffset += sizeof(" + GetCPPType(fieldInfo.FieldTypename) + ");");
                        }
                    }

                    textWriter.WriteLine("\t\t}");

                    if (classInfo.Metadata.ContainsKey("[handler]"))
                    {
                        textWriter.WriteLine();
                        textWriter.WriteLine("\t\tint GetProcessingChannel() override { return RouteID::" + classInfo.Metadata["[handler]"] + "; }");
                    }

                    // offsets
                    textWriter.WriteLine();
                    foreach (FieldInfo fieldInfo in classInfo.Fields)
                    {
                        textWriter.WriteLine("\t\tsize_t " + fieldInfo.Name + "Offset = 0;");
                    }

                    foreach (FieldInfo fieldInfo in classInfo.Fields)
                    {
                        textWriter.WriteLine();

                        string offset = fieldInfo.Name + "Offset";

                        if (fieldInfo.FieldTypename == "string")
                        {
                            textWriter.WriteLine("\t\tstd::string_view " + fieldInfo.Name + "() const { return ReadString(" + offset + "); }");
                            textWriter.WriteLine("\t\tvoid Set" + fieldInfo.Name + "(std::string_view value) { WriteBufferValue(value.data(), value.size(), " + offset + "); }");
                        }
                        else
                        {
                            textWriter.WriteLine("\t\t" + GetCPPType(fieldInfo.FieldTypename) + " " + fieldInfo.Name + "() const { return *ReadValue<" + GetCPPType(fieldInfo.FieldTypename) + ">(" + offset + "); }");
                            textWriter.WriteLine("\t\tvoid Set" + fieldInfo.Name + "(" + GetCPPType(fieldInfo.FieldTypename) + " value) { WriteValue<" + GetCPPType(fieldInfo.FieldTypename) + ">(value, " + offset + "); }");
                        }
                    }

                    textWriter.WriteLine("\t};");
                    textWriter.WriteLine();
                }
            }

            if (!string.IsNullOrEmpty(namespaceName))
            {
                textWriter.WriteLine("} //" + namespaceName);
            }

            textWriter.Flush();
            textWriter.Close();
        }
    }
}
