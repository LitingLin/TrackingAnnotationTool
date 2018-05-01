using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;

namespace web_service.Controllers
{
    [Produces("application/json")]
    [Route("api/annotations")]
    public class AnnotationsController : Controller
    {
        private IConfiguration _configuration;

        public AnnotationsController(IConfiguration Configuration)
        {
            _configuration = Configuration;
        }
        public struct Record
        {
            public bool labeled;
            public uint x;
            public uint y;
            public uint w;
            public uint h;
            public bool occlusion;
            public bool outOfView;
        }

        private string[] GetSequences()
        {
            string datasetPath = _configuration["DataSet:Path"];
            DirectoryInfo datasetDirectoryInfo = new DirectoryInfo(datasetPath);
            var dirs = datasetDirectoryInfo.EnumerateDirectories();
            IList<string> sequences = new List<string>();
            foreach (var dir in dirs)
            {
                DirectoryInfo subDirectoryInfo = new DirectoryInfo(dir.FullName);
                var subDirs = subDirectoryInfo.EnumerateDirectories();
                foreach (var subdir in subDirs)
                {
                    sequences.Add(dir.Name + '/' + subdir.Name);
                }
            }

            return sequences.ToArray();
        }

        // GET api/values
        [HttpGet]
        public IEnumerable<string> Get()
        {
            return GetSequences();
        }

        [HttpGet]
        [Route("{sequence}/{subSequence}")]
        public IEnumerable<Record> Get(string sequence, string subSequence)
        {
            return new Record[]{new Record(), };
        }
    }
}