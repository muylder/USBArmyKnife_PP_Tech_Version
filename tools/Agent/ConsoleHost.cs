using Agent;

// Simple console host for the Agent DLL
// This allows us to run it with "dotnet run"

class Program
{
    static async Task Main(string[] args)
    {
        // If no args, use defaults
        if (args.Length == 0)
        {
            args = new[] { "vid=303A", "pid=1001" };
            Console.WriteLine("Using default VID/PID for ESP32-S3");
        }

        // Call the Agent's main
        await Agent.Program.Main(args);
    }
}
